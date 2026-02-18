/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coders.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alyousse <alyousse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/16 13:54:27 by alyousse          #+#    #+#             */
/*   Updated: 2026/02/18 08:48:28 by alyousse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "coders.h"
#include "parser/parse.h"
#include <unistd.h>
#include "logs/log.h"
#include "stop/stop.h"


#include <unistd.h> // usleep

static int	get_compile_count(t_coder *c)
{
	int	v;

	pthread_mutex_lock(&c->m);
	v = c->compile_count;
	pthread_mutex_unlock(&c->m);
	return v;
}

void	*coder_routine(void *arg)
{
	t_coder	*c;
	long	now;

	c = (t_coder *)arg;
	while (!sim_should_stop(c->sim) &&
			get_compile_count(c) < c->sim->config.required_compiles)
	{
		// mark compile start (for burnout monitor later)
		now = get_time_ms();
		pthread_mutex_lock(&c->m);
		c->last_compile_start_ms = now;
		pthread_mutex_unlock(&c->m);

		log_state(c->sim, c->id, "is compiling");
		usleep(c->sim->config.time_to_compile * 1000);

		// increment compile count
		pthread_mutex_lock(&c->m);
		c->compile_count++;
		pthread_mutex_unlock(&c->m);

		log_state(c->sim, c->id, "is debugging");
		usleep(c->sim->config.time_to_debug * 1000);

		log_state(c->sim, c->id, "is refactoring");
		usleep(c->sim->config.time_to_refactor * 1000);
	}

	log_state(c->sim, c->id, "exiting");
	return (NULL);
}


static int	all_done(t_sim *sim)
{
	int i;

	i = 0;
	while (i < sim->config.nb_of_coders)
	{
		pthread_mutex_lock(&sim->coders[i].m);
		if (sim->coders[i].compile_count < sim->config.required_compiles)
		{
			pthread_mutex_unlock(&sim->coders[i].m);
			return (0);
		}
		pthread_mutex_unlock(&sim->coders[i].m);
		i++;
	}
	return (1);
}

void	*monitor_routine(void *arg)
{
	t_sim	*sim;
	long	now;
	long	last_start;
	int		i;

	sim = (t_sim *)arg; // its being casted to t_sim because the pthread_creates() expects a function with void argument
	while (!sim_should_stop(sim)) // check if it should stop each time
	{
		now = get_time_ms();
		// Burnout detection (must log within 10ms)
		i = 0;
		while (i < sim->config.nb_of_coders) // loop for all coders
		{
			pthread_mutex_lock(&sim->coders[i].m); // lock each coder at a time
			last_start = sim->coders[i].last_compile_start_ms; // updates last start for each coder (its calculates in coder routine)
			pthread_mutex_unlock(&sim->coders[i].m); // unlock
			if (now - last_start > sim->config.time_to_burnout) // calculate if its burn out , it should be less than 10 milliseconds
			{
				log_state(sim, sim->coders[i].id, "burned out"); // log burn out
				sim_set_stop(sim); // stop the simulation
				return (NULL);
			}
			i++;
		}
		if (all_done(sim)) // on done
		{
			sim_set_stop(sim); // also stop
			return (NULL);
		}
		usleep(1000); // preventing cpu overload
	}
	return (NULL);
}
