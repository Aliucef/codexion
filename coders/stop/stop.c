#include "stop.h"
#include "../parser/parse.h"




int	sim_should_stop(t_sim *sim) // again sim is the simulation struct and it contains all of our world data, and this function is used to check if the simulation should stop or not
{
	int	val;

	pthread_mutex_lock(&sim->stop_m); // locks stop mutex
	val = sim->stop; // reads the value of stop, if it's 1 then we should stop the simulation, if it's 0 then we should continue 
	pthread_mutex_unlock(&sim->stop_m); // unlocks stop mutex
	return (val);
}

void	sim_set_stop(t_sim *sim) // this function is used to set the stop flag to 1, which will signal all threads that they should stop the simulation
{
	pthread_mutex_lock(&sim->stop_m);
	sim->stop = 1;
	pthread_mutex_unlock(&sim->stop_m);
}
