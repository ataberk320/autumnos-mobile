#include <time.h>
#include <stdbool.h>
#include "cheaders/AutumnAPPSSTimer.h"
typedef struct {
	time_t last_seen;
	bool is_running;
} AppSession;

AppSession sessions[10];

bool is_session_valid(int app_index) {
	if (!sessions[app_index].is_running) return false;

	time_t now = time(NULL);
	double diff = difftime(now, sessions[app_index].last_seen);
	return diff < 1800;
}

void update_session(int app_index) {
	sessions[app_index].last_seen = time(NULL);
	sessions[app_index].is_running = true;
}
