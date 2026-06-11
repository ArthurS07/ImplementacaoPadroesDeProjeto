/**
 * Padrão Criacional: Factory Method
 * 
 * Contexto: Um sistema de notificações precisa enviar alertas por diferentes
 * canais (Email, SMS, Push). O código cliente não deve depender das classes
 * concretas de cada notificador.
 * 
 * Problema: Como criar objetos de notificação sem acoplar o código ao tipo
 * específico de notificador?
 * 
 * Solução: Definir uma interface (Notifier) e uma função criadora (factory)
 * que decide qual tipo concreto instanciar em tempo de execução.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ── Tipos de notificador ── */
typedef enum {
    NOTIFIER_EMAIL,
    NOTIFIER_SMS,
    NOTIFIER_PUSH
} NotifierType;

/* ── Interface (simulada com ponteiro de função) ── */
typedef struct Notifier {
    void (*send)(struct Notifier *self, const char *message);
    void (*destroy)(struct Notifier *self);
    char type_name[16];
} Notifier;

/* ── Implementação: Email ── */
typedef struct {
    Notifier base;
    char address[64];
} EmailNotifier;

static void email_send(Notifier *self, const char *message) {
    EmailNotifier *en = (EmailNotifier *)self;
    printf("[EMAIL] Para: %s | Mensagem: %s\n", en->address, message);
}

static void email_destroy(Notifier *self) {
    free(self);
}

Notifier *create_email_notifier(const char *address) {
    EmailNotifier *en = malloc(sizeof(EmailNotifier));
    en->base.send    = email_send;
    en->base.destroy = email_destroy;
    strncpy(en->base.type_name, "Email", sizeof(en->base.type_name) - 1);
    strncpy(en->address, address, sizeof(en->address) - 1);
    return (Notifier *)en;
}

/* ── Implementação: SMS ── */
typedef struct {
    Notifier base;
    char phone[16];
} SmsNotifier;

static void sms_send(Notifier *self, const char *message) {
    SmsNotifier *sn = (SmsNotifier *)self;
    printf("[SMS]   Para: %s | Mensagem: %s\n", sn->phone, message);
}

static void sms_destroy(Notifier *self) {
    free(self);
}

Notifier *create_sms_notifier(const char *phone) {
    SmsNotifier *sn = malloc(sizeof(SmsNotifier));
    sn->base.send    = sms_send;
    sn->base.destroy = sms_destroy;
    strncpy(sn->base.type_name, "SMS", sizeof(sn->base.type_name) - 1);
    strncpy(sn->phone, phone, sizeof(sn->phone) - 1);
    return (Notifier *)sn;
}

/* ── Implementação: Push ── */
typedef struct {
    Notifier base;
    char device_id[32];
} PushNotifier;

static void push_send(Notifier *self, const char *message) {
    PushNotifier *pn = (PushNotifier *)self;
    printf("[PUSH]  Device: %s | Mensagem: %s\n", pn->device_id, message);
}

static void push_destroy(Notifier *self) {
    free(self);
}

Notifier *create_push_notifier(const char *device_id) {
    PushNotifier *pn = malloc(sizeof(PushNotifier));
    pn->base.send    = push_send;
    pn->base.destroy = push_destroy;
    strncpy(pn->base.type_name, "Push", sizeof(pn->base.type_name) - 1);
    strncpy(pn->device_id, device_id, sizeof(pn->device_id) - 1);
    return (Notifier *)pn;
}

/* ── Factory Method ── */
Notifier *notifier_factory(NotifierType type, const char *target) {
    switch (type) {
        case NOTIFIER_EMAIL: return create_email_notifier(target);
        case NOTIFIER_SMS:   return create_sms_notifier(target);
        case NOTIFIER_PUSH:  return create_push_notifier(target);
        default:             return NULL;
    }
}

/* ── Código cliente ── */
int main(void) {
    printf("=== Factory Method: Sistema de Notificações ===\n\n");

    /* O cliente usa apenas o tipo genérico Notifier* — não conhece os concretos */
    Notifier *notifiers[3];
    notifiers[0] = notifier_factory(NOTIFIER_EMAIL, "usuario@email.com");
    notifiers[1] = notifier_factory(NOTIFIER_SMS,   "+5544999990000");
    notifiers[2] = notifier_factory(NOTIFIER_PUSH,  "device-abc-123");

    const char *alerta = "Sua compra foi aprovada!";

    for (int i = 0; i < 3; i++) {
        notifiers[i]->send(notifiers[i], alerta);
        notifiers[i]->destroy(notifiers[i]);
    }

    printf("\nTodos os notificadores liberados com sucesso.\n");
    return 0;
}
