/**
 * Padrão Estrutural: Adapter
 * 
 * Contexto: Um sistema de pagamentos já usa uma interface interna (PaymentProcessor).
 * Precisamos integrar duas gateways externas legadas — PagSeguro e Stripe — que
 * possuem APIs completamente diferentes.
 * 
 * Problema: Como reutilizar código legado (incompatível) sem modificá-lo?
 * 
 * Solução: Criar adaptadores que "traduzem" a interface legada para a interface
 * esperada pelo sistema, sem alterar nenhum dos dois lados.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ─────────────────────────────────────────
   Interface alvo (usada pelo sistema)
   ───────────────────────────────────────── */
typedef struct PaymentProcessor {
    int (*process_payment)(struct PaymentProcessor *self,
                           const char *order_id, double amount);
    void (*destroy)(struct PaymentProcessor *self);
} PaymentProcessor;

/* ─────────────────────────────────────────
   Biblioteca legada 1: PagSeguro (API própria)
   ───────────────────────────────────────── */
typedef struct {
    char merchant_token[64];
} PagSeguroSDK;

static int pagseguro_realizar_cobranca(PagSeguroSDK *sdk,
                                        const char *pedido,
                                        float valor_reais) {
    printf("[PagSeguro SDK] Token: %.8s... | Pedido: %s | Valor: R$ %.2f\n",
           sdk->merchant_token, pedido, valor_reais);
    return 1; /* sucesso */
}

/* ─────────────────────────────────────────
   Biblioteca legada 2: Stripe (API própria)
   ───────────────────────────────────────── */
typedef struct {
    char secret_key[64];
    char currency[4];
} StripeClient;

static int stripe_charge(StripeClient *client,
                          int amount_cents, const char *description) {
    printf("[Stripe Client] Key: %.8s... | %s | %d centavos (%s)\n",
           client->secret_key, description, amount_cents, client->currency);
    return 200; /* HTTP-like status OK */
}

/* ─────────────────────────────────────────
   Adapter 1: PagSeguro → PaymentProcessor
   ───────────────────────────────────────── */
typedef struct {
    PaymentProcessor base;
    PagSeguroSDK     sdk;
} PagSeguroAdapter;

static int pagseguro_adapter_process(PaymentProcessor *self,
                                      const char *order_id, double amount) {
    PagSeguroAdapter *a = (PagSeguroAdapter *)self;
    return pagseguro_realizar_cobranca(&a->sdk, order_id, (float)amount);
}

static void pagseguro_adapter_destroy(PaymentProcessor *self) {
    free(self);
}

PaymentProcessor *create_pagseguro_adapter(const char *token) {
    PagSeguroAdapter *a = malloc(sizeof(PagSeguroAdapter));
    a->base.process_payment = pagseguro_adapter_process;
    a->base.destroy         = pagseguro_adapter_destroy;
    strncpy(a->sdk.merchant_token, token, sizeof(a->sdk.merchant_token) - 1);
    return (PaymentProcessor *)a;
}

/* ─────────────────────────────────────────
   Adapter 2: Stripe → PaymentProcessor
   ───────────────────────────────────────── */
typedef struct {
    PaymentProcessor base;
    StripeClient     client;
} StripeAdapter;

static int stripe_adapter_process(PaymentProcessor *self,
                                   const char *order_id, double amount) {
    StripeAdapter *a = (StripeAdapter *)self;
    int cents = (int)(amount * 100);
    int status = stripe_charge(&a->client, cents, order_id);
    return (status == 200) ? 1 : 0;
}

static void stripe_adapter_destroy(PaymentProcessor *self) {
    free(self);
}

PaymentProcessor *create_stripe_adapter(const char *secret_key,
                                         const char *currency) {
    StripeAdapter *a = malloc(sizeof(StripeAdapter));
    a->base.process_payment = stripe_adapter_process;
    a->base.destroy         = stripe_adapter_destroy;
    strncpy(a->client.secret_key, secret_key, sizeof(a->client.secret_key) - 1);
    strncpy(a->client.currency,   currency,   sizeof(a->client.currency)   - 1);
    return (PaymentProcessor *)a;
}

/* ─────────────────────────────────────────
   Código cliente (usa apenas PaymentProcessor*)
   ───────────────────────────────────────── */
static void checkout(PaymentProcessor *processor,
                     const char *order_id, double amount) {
    printf(">> Processando pedido '%s' — R$ %.2f\n", order_id, amount);
    int ok = processor->process_payment(processor, order_id, amount);
    printf("   Resultado: %s\n\n", ok ? "APROVADO" : "RECUSADO");
}

int main(void) {
    printf("=== Adapter: Gateways de Pagamento ===\n\n");

    PaymentProcessor *ps = create_pagseguro_adapter("TOKEN_PAGSEGURO_XYZ");
    PaymentProcessor *st = create_stripe_adapter("sk_test_STRIPE_KEY", "BRL");

    checkout(ps, "PEDIDO-001", 149.90);
    checkout(st, "PEDIDO-002", 299.00);

    ps->destroy(ps);
    st->destroy(st);

    printf("Processadores liberados.\n");
    return 0;
}
