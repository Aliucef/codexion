#include <stdio.h>
#include "../parser/parse.h"

void	log_state(t_sim *sim, int coder_id, const char *message)
{
	long	now;
	long	timestamp;

	if (!sim || !message)
		return ;
    
	now = get_time_ms(); // This will get the current time in milliseconds
	timestamp = now - sim->start_ms; // Calculate the timestamp relative to the simulation start time
	pthread_mutex_lock(&sim->log_m); // Lock the log mutex to ensure thread-safe logging
	printf("%ld %d %s\n", timestamp, coder_id, message); // print the messsage
	pthread_mutex_unlock(&sim->log_m); // unlock the log mutex
}
