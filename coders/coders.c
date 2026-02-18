/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coders.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alyousse <alyousse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/16 13:54:27 by alyousse          #+#    #+#             */
/*   Updated: 2026/02/18 12:06:46 by alyousse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "coders.h"
#include "parser/parse.h"
#include <unistd.h>
#include "logs/log.h"
#include "stop/stop.h"
#include <unistd.h> // usleep
#include "dongles/dongles.h"
static int	get_compile_count(t_coder *c)
{
	int	v;

	pthread_mutex_lock(&c->m);
	v = c->compile_count;
	pthread_mutex_unlock(&c->m);
	return v;
}

void	sim_wake_all(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->config.nb_of_coders)
	{
		pthread_mutex_lock(&sim->dongles[i].mutex);
		pthread_cond_broadcast(&sim->dongles[i].condvar);
		pthread_mutex_unlock(&sim->dongles[i].mutex);
		i++;
	}
}


void	*coder_routine(void *arg)
{
	t_coder	*coder;
	long	now;

	coder = (t_coder *)arg;
	int n = coder->sim->config.nb_of_coders;
	int left = coder->id - 1;
	int right = coder->id % n;
	int first = (left < right) ? left : right;
	int second = (left < right) ? right : left;

	while (!sim_should_stop(coder->sim) &&
			get_compile_count(coder) < coder->sim->config.required_compiles)
	{
		if (!dongle_take(coder->sim, first, coder->id))
			break;
		if (!dongle_take(coder->sim, second, coder->id))
		{
			dongle_release(coder->sim, first);
			break;
		}
		now = get_time_ms();
		pthread_mutex_lock(&coder->m);
		coder->last_compile_start_ms = now;
		pthread_mutex_unlock(&coder->m);
		log_state(coder->sim, coder->id, "is compiling");
		usleep(coder->sim->config.time_to_compile * 1000);
		dongle_release(coder->sim, first);
		dongle_release(coder->sim, second);
		pthread_mutex_lock(&coder->m);
		coder->compile_count++;
		pthread_mutex_unlock(&coder->m);
		log_state(coder->sim, coder->id, "is debugging");
		usleep(coder->sim->config.time_to_debug * 1000);
		log_state(coder->sim, coder->id, "is refactoring");
		usleep(coder->sim->config.time_to_refactor * 1000);
	}
	log_state(coder->sim, coder->id, "exiting");
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
				sim_wake_all(sim);
				return (NULL);
			}
			i++;
		}
		if (all_done(sim)) // on done
		{
			sim_set_stop(sim); // also stop
			sim_wake_all(sim);
			return (NULL);
		}
		usleep(1000); // preventing cpu overload
	}
	return (NULL);
}
