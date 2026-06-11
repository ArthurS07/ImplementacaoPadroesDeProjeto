# Padrões de Projeto em C

Implementação de três padrões de projeto clássicos em **linguagem C**, baseados no catálogo do [Refactoring Guru](https://refactoring.guru/pt-br/design-patterns).

> **Aviso sobre uso de IA:** Os exemplos de código e explicações neste repositório foram elaborados com auxílio da ferramenta **Claude (Anthropic)**.

---

## Estrutura do Repositório

```
design-patterns/
├── factory_method/
│   └── factory_method.c
├── adapter/
│   └── adapter.c
├── observer/
│   └── observer.c
└── README.md
```

---

## Como Compilar e Executar

É necessário um compilador C (GCC ou Clang). Exemplo com GCC:

```bash
gcc factory_method/factory_method.c -o factory_method
./factory_method

gcc adapter/adapter.c -o adapter
./adapter

gcc observer/observer.c -o observer
./observer
```

---

## 1. Factory Method — Padrão Criacional

**Referência:** [Refactoring Guru — Factory Method](https://refactoring.guru/pt-br/design-patterns/factory-method)

### Contexto

Um sistema de notificações precisa enviar alertas por diferentes canais: **e-mail**, **SMS** e **notificação push**. O código principal não deve depender diretamente de nenhum tipo específico de notificador.

### Problema

Criar objetos de notificação diretamente com `malloc` espalhado pelo código-cliente cria acoplamento forte: qualquer adição de novo canal exige modificar o código cliente. Isso viola o **Princípio Aberto/Fechado**.

### Solução

Definir uma interface genérica `Notifier` com ponteiros de função (`send`, `destroy`) e uma **factory function** `notifier_factory()` que recebe um tipo enumerado e retorna o ponteiro para a interface. O cliente trabalha apenas com `Notifier*`, sem nunca conhecer `EmailNotifier`, `SmsNotifier` ou `PushNotifier`.

### Diagrama de Estrutura

```
         notifier_factory(type, target)
                    │
         ┌──────────┼──────────┐
         ▼          ▼          ▼
  EmailNotifier  SmsNotifier  PushNotifier
         │          │          │
         └──────────┴──────────┘
                    │
               Notifier*
              .send()
              .destroy()
```

### Código Explicado

```c
// Interface genérica — o cliente só conhece isso
typedef struct Notifier {
    void (*send)(struct Notifier *self, const char *message);
    void (*destroy)(struct Notifier *self);
    char type_name[16];
} Notifier;

// Factory: decide qual concreto criar
Notifier *notifier_factory(NotifierType type, const char *target) {
    switch (type) {
        case NOTIFIER_EMAIL: return create_email_notifier(target);
        case NOTIFIER_SMS:   return create_sms_notifier(target);
        case NOTIFIER_PUSH:  return create_push_notifier(target);
        default:             return NULL;
    }
}

// Uso pelo cliente — sem saber qual tipo concreto é
Notifier *n = notifier_factory(NOTIFIER_EMAIL, "user@email.com");
n->send(n, "Sua compra foi aprovada!");
n->destroy(n);
```

### Saída Esperada

```
[EMAIL] Para: usuario@email.com | Mensagem: Sua compra foi aprovada!
[SMS]   Para: +5544999990000 | Mensagem: Sua compra foi aprovada!
[PUSH]  Device: device-abc-123 | Mensagem: Sua compra foi aprovada!
```

---

## 2. Adapter — Padrão Estrutural

**Referência:** [Refactoring Guru — Adapter](https://refactoring.guru/pt-br/design-patterns/adapter)

### Contexto

Um sistema de pagamentos usa internamente a interface `PaymentProcessor`. É necessário integrar dois SDKs legados — **PagSeguro** e **Stripe** — que possuem APIs completamente distintas entre si e incompatíveis com a interface interna.

### Problema

Modificar os SDKs externos não é viável (são bibliotecas de terceiros). Reescrever o sistema interno para cada SDK cria duplicação e rompe a arquitetura. É preciso integrar sem alterar nenhum dos lados.

### Solução

Criar um **adaptador** para cada SDK. Cada adaptador implementa a interface `PaymentProcessor` (alvo) e delega a chamada internamente para a API específica do SDK, realizando a "tradução" de tipos e parâmetros necessária.

### Diagrama de Estrutura

```
  Código Cliente
       │
       ▼
  PaymentProcessor*          ← interface alvo
  .process_payment()
       │
  ┌────┴────┐
  ▼         ▼
PagSeguro  Stripe
Adapter    Adapter          ← adaptadores
  │              │
  ▼              ▼
PagSeguroSDK   StripeClient  ← SDKs legados (inalterados)
.realizar_     .charge()
 cobranca()
```

### Código Explicado

```c
// Interface alvo que o sistema espera
typedef struct PaymentProcessor {
    int (*process_payment)(struct PaymentProcessor *self,
                           const char *order_id, double amount);
    void (*destroy)(struct PaymentProcessor *self);
} PaymentProcessor;

// Adaptador que "traduz" para o PagSeguro
static int pagseguro_adapter_process(PaymentProcessor *self,
                                      const char *order_id, double amount) {
    PagSeguroAdapter *a = (PagSeguroAdapter *)self;
    // Converte double → float (diferença de API)
    return pagseguro_realizar_cobranca(&a->sdk, order_id, (float)amount);
}

// Adaptador que "traduz" para o Stripe
static int stripe_adapter_process(PaymentProcessor *self,
                                   const char *order_id, double amount) {
    StripeAdapter *a = (StripeAdapter *)self;
    int cents = (int)(amount * 100); // converte reais → centavos
    int status = stripe_charge(&a->client, cents, order_id);
    return (status == 200) ? 1 : 0; // normaliza retorno HTTP → bool
}
```

### Saída Esperada

```
>> Processando pedido 'PEDIDO-001' — R$ 149.90
[PagSeguro SDK] Token: TOKEN_PA... | Pedido: PEDIDO-001 | Valor: R$ 149.90
   Resultado: APROVADO

>> Processando pedido 'PEDIDO-002' — R$ 299.00
[Stripe Client] Key: sk_test_... | PEDIDO-002 | 29900 centavos (BRL)
   Resultado: APROVADO
```

---

## 3. Observer — Padrão Comportamental

**Referência:** [Refactoring Guru — Observer](https://refactoring.guru/pt-br/design-patterns/observer)

### Contexto

Uma **estação meteorológica** coleta leituras de temperatura. Múltiplos componentes precisam reagir a cada nova medição: um display visual, um sistema de alertas e um logger de dados.

### Problema

Chamar cada componente diretamente dentro da estação cria acoplamento rígido. Adicionar ou remover um componente obriga a modificar a classe da estação, o que viola novamente o **Princípio Aberto/Fechado**.

### Solução

Implementar a relação **publicador–assinante**: a `WeatherStation` mantém uma lista de `Observer*` e os notifica automaticamente via `update()` a cada mudança. Observers podem se inscrever e cancelar dinamicamente, e a estação nunca precisa conhecer os tipos concretos.

### Diagrama de Estrutura

```
  WeatherStation (Subject/Publisher)
  ┌──────────────────────────────┐
  │ observers[]: Observer*[]     │
  │ subscribe(obs)               │
  │ unsubscribe(obs)             │
  │ set_temperature() → notify() │
  └──────────────┬───────────────┘
                 │ update(temperature)
     ┌───────────┼───────────┐
     ▼           ▼           ▼
 Temperature  AlertSystem  DataLogger
  Display     (threshold)  (contador)
```

### Código Explicado

```c
// Interface Observer — todo assinante implementa update()
typedef struct Observer {
    void (*update)(struct Observer *self, float temperature);
    char name[32];
} Observer;

// Subject notifica todos os inscritos
static void station_notify(WeatherStation *ws) {
    for (int i = 0; i < ws->count; i++) {
        ws->observers[i]->update(ws->observers[i], ws->temperature);
    }
}

// Mudança de estado dispara notificação automática
void station_set_temperature(WeatherStation *ws, float temp) {
    ws->temperature = temp;
    station_notify(ws);  // todos os observers são avisados
}

// Observer concreto: alerta se temperatura exceder o limite
static void alert_update(Observer *self, float temperature) {
    AlertSystem *a = (AlertSystem *)self;
    if (temperature > a->threshold) {
        printf("[ALERTA!] %.1f°C acima do limite!\n", temperature);
    }
}
```

### Saída Esperada

```
[Estação] Nova leitura: 36.8°C — notificando 3 observer(s)...
  [Display]  Temperatura atual: 36.8°C
  [ALERTA!]  Temperatura 36.8°C acima do limite (35.0°C)!
  [Logger]   Leitura #2 registrada: 36.8°C
```

---

## Referências

- Refactoring Guru — *Design Patterns*: https://refactoring.guru/pt-br/design-patterns  
  - [Factory Method](https://refactoring.guru/pt-br/design-patterns/factory-method)  
  - [Adapter](https://refactoring.guru/pt-br/design-patterns/adapter)  
  - [Observer](https://refactoring.guru/pt-br/design-patterns/observer)  

O conteúdo conceitual (definições, intenções e estrutura dos padrões) é baseado no material do Refactoring Guru, referenciado acima conforme exigido por questões de direitos autorais.

---

> **LLM utilizada:** Claude (Anthropic) — utilizado para auxiliar na elaboração dos exemplos de código e explicações deste repositório.
