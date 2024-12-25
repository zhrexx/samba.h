#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_EVENTS 10

typedef struct {
    char *event_id;
    int status; // 0 = Not called, 1 = called
    void (*callback)(void);
} Event;

typedef struct {
    Event *events;
    int count_events;
} P_EventManager;

static P_EventManager *p_event_manager;

int p_init_event_manager(void) {
    p_event_manager = (P_EventManager *)malloc(sizeof(P_EventManager));
    if (p_event_manager == NULL) {
        return 0;
    }
    p_event_manager->count_events = 0;
    p_event_manager->events = (Event *)malloc(sizeof(Event) * MAX_EVENTS);
    if (p_event_manager->events == NULL) {
        free(p_event_manager);
        return 0;
    }
    return 1;
}

int p_register_event(const char *event_id, void (*callback)(void)) {
    if (p_event_manager->count_events >= MAX_EVENTS) {
        return 0;
    }
    Event event;
    event.event_id = strdup(event_id);
    if (event.event_id == NULL) {
        return 0;
    }
    event.status = 0;
    event.callback = callback;
    p_event_manager->events[p_event_manager->count_events] = event;
    p_event_manager->count_events++;
    return 1;
}

int p_call_event(const char *event_id) {
    for (int i = 0; i < p_event_manager->count_events; i++) {
        if (strcmp(p_event_manager->events[i].event_id, event_id) == 0) {
            if (p_event_manager->events[i].status == 0) {
                p_event_manager->events[i].status = 1;
                if (p_event_manager->events[i].callback == NULL) {
                    return 1;
                }
                else {
                    p_event_manager->events[i].callback();
                    return 1;
                }

            } else {
                return 0;
            }
        }
    }
    return 0;
}

P_EventManager *p_get_event_manager(void) {
    return p_event_manager;
}

int p_execute_event(const char *event_id) {
    P_EventManager *event_manager = p_get_event_manager();
    for (int i = 0; i < event_manager->count_events; i++) {
        if (strcmp(event_manager->events[i].event_id, event_id) == 0) {
            event_manager->events[i].callback();
            return 1;
        }
    }
    return 0;
}

int p_get_event_status(const char *event_id) {
    P_EventManager *event_manager = p_get_event_manager();
    for (int i = 0; i < event_manager->count_events; i++) {
        if (strcmp(event_manager->events[i].event_id, event_id) == 0) {
            return event_manager->events[i].status;
        }
    }
    return -1;
}




int p_init(void) {
    // Ran Event (autocall on init)
    if (!p_init_event_manager()) {
        return 0;
    }
    if (!p_register_event("ran", NULL)) {
        return 0;
    }
    p_call_event("ran");

    return 1;
}
