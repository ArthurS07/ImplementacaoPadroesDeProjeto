/**
 * Padrão Comportamental: Observer
 * 
 * Contexto: Uma estação meteorológica coleta dados de temperatura.
 * Diferentes componentes (display, sistema de alertas, logger) precisam
 * ser atualizados automaticamente sempre que uma nova leitura ocorre.
 * 
 * Problema: Como notificar múltiplos objetos sobre mudanças de estado
 * sem criar dependências rígidas entre eles?
 * 
 * Solução: Definir uma relação publicador–assinante. O Subject (estação)
 * mantém uma lista de Observers e os notifica automaticamente a cada
 * mudança, sem conhecer os tipos concretos.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_OBSERVERS 8

/* ─────────────────────────────────────────
   Interface Observer
   ───────────────────────────────────────── */
typedef struct Observer {
    void (*update)(struct Observer *self, float temperature);
    char name[32];
} Observer;

/* ─────────────────────────────────────────
   Subject: Estação Meteorológica
   ───────────────────────────────────────── */
typedef struct {
    Observer *observers[MAX_OBSERVERS];
    int       count;
    float     temperature;
} WeatherStation;

void station_init(WeatherStation *ws) {
    ws->count       = 0;
    ws->temperature = 0.0f;
}

void station_subscribe(WeatherStation *ws, Observer *obs) {
    if (ws->count < MAX_OBSERVERS) {
        ws->observers[ws->count++] = obs;
        printf("[Estação] '%s' inscrito.\n", obs->name);
    }
}

void station_unsubscribe(WeatherStation *ws, Observer *obs) {
    for (int i = 0; i < ws->count; i++) {
        if (ws->observers[i] == obs) {
            ws->observers[i] = ws->observers[--ws->count];
            printf("[Estação] '%s' removido.\n", obs->name);
            return;
        }
    }
}

static void station_notify(WeatherStation *ws) {
    for (int i = 0; i < ws->count; i++) {
        ws->observers[i]->update(ws->observers[i], ws->temperature);
    }
}

void station_set_temperature(WeatherStation *ws, float temp) {
    printf("\n[Estação] Nova leitura: %.1f°C — notificando %d observer(s)...\n",
           temp, ws->count);
    ws->temperature = temp;
    station_notify(ws);
}

/* ─────────────────────────────────────────
   Observer concreto 1: Display de Temperatura
   ───────────────────────────────────────── */
typedef struct {
    Observer base;
} TemperatureDisplay;

static void display_update(Observer *self, float temperature) {
    printf("  [Display]  Temperatura atual: %.1f°C\n", temperature);
}

Observer *create_display(void) {
    TemperatureDisplay *d = malloc(sizeof(TemperatureDisplay));
    d->base.update = display_update;
    strncpy(d->base.name, "Display", sizeof(d->base.name) - 1);
    return (Observer *)d;
}

/* ─────────────────────────────────────────
   Observer concreto 2: Sistema de Alertas
   ───────────────────────────────────────── */
typedef struct {
    Observer base;
    float    threshold;
} AlertSystem;

static void alert_update(Observer *self, float temperature) {
    AlertSystem *a = (AlertSystem *)self;
    if (temperature > a->threshold) {
        printf("  [ALERTA!]  Temperatura %.1f°C acima do limite (%.1f°C)!\n",
               temperature, a->threshold);
    } else {
        printf("  [Alertas]  Temperatura dentro do limite normal.\n");
    }
}

Observer *create_alert_system(float threshold) {
    AlertSystem *a = malloc(sizeof(AlertSystem));
    a->base.update = alert_update;
    a->threshold   = threshold;
    strncpy(a->base.name, "Alertas", sizeof(a->base.name) - 1);
    return (Observer *)a;
}

/* ─────────────────────────────────────────
   Observer concreto 3: Logger
   ───────────────────────────────────────── */
typedef struct {
    Observer base;
    int      reading_count;
} DataLogger;

static void logger_update(Observer *self, float temperature) {
    DataLogger *l = (DataLogger *)self;
    l->reading_count++;
    printf("  [Logger]   Leitura #%d registrada: %.1f°C\n",
           l->reading_count, temperature);
}

Observer *create_logger(void) {
    DataLogger *l = malloc(sizeof(DataLogger));
    l->base.update   = logger_update;
    l->reading_count = 0;
    strncpy(l->base.name, "Logger", sizeof(l->base.name) - 1);
    return (Observer *)l;
}

/* ─────────────────────────────────────────
   Código cliente
   ───────────────────────────────────────── */
int main(void) {
    printf("=== Observer: Estação Meteorológica ===\n\n");

    WeatherStation station;
    station_init(&station);

    Observer *display = create_display();
    Observer *alerts  = create_alert_system(35.0f);
    Observer *logger  = create_logger();

    station_subscribe(&station, display);
    station_subscribe(&station, alerts);
    station_subscribe(&station, logger);

    station_set_temperature(&station, 22.5f);
    station_set_temperature(&station, 36.8f); /* acima do limite */
    station_set_temperature(&station, 18.0f);

    printf("\n--- Removendo o sistema de alertas ---\n");
    station_unsubscribe(&station, alerts);

    station_set_temperature(&station, 40.0f); /* alertas não recebem mais */

    free(display);
    free(alerts);
    free(logger);

    printf("\nObjetos liberados. Fim.\n");
    return 0;
}
