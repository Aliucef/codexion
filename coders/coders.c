/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coders.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alyousse <alyousse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/16 13:54:27 by alyousse          #+#    #+#             */
/*   Updated: 2026/02/18 13:24:39 by alyousse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "coders.h"
#include "parser/parse.h"
#include <unistd.h>
#include "logs/log.h"
#include "stop/stop.h"
#include <unistd.h>
#include "dongles/dongles.h"

static int	get_compile_count(t_coder *c)
{
	int	v;

	pthread_mutex_lock(&c->m);
	v = c->compile_count;
	pthread_mutex_unlock(&c->m);
	return (v);
}

void	sim_wake_all(t_sim *sim)
{
	int	i;

	i = -1;
	while (++i < sim->config.nb_of_coders)
	{
		pthread_mutex_lock(&sim->dongles[i].mutex);
		pthread_cond_broadcast(&sim->dongles[i].condvar);
		pthread_mutex_unlock(&sim->dongles[i].mutex);
	}
}

void	*coder_routine(void *arg)
{
	t_coder	*coder;
	long	now;

	coder = (t_coder *)arg;
	int	n = coder->sim->config.nb_of_coders;
	int	left = coder->id - 1;
	int	right = coder->id % n;
	int	first = (left < right) ? left : right;
	int	second = (left < right) ? right : left;

	while (!sim_should_stop(coder->sim) &&
			get_compile_count(coder) < coder->sim->config.required_compiles)
	{
		if (!dongle_take(coder->sim, first, coder->id))
			break ;
		if (!dongle_take(coder->sim, second, coder->id))
		{
			dongle_release(coder->sim, first);
			break ;
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
	int	i;

	i = -1;
	while (++i < sim->config.nb_of_coders)
	{
		pthread_mutex_lock(&sim->coders[i].m);
		if (sim->coders[i].compile_count < sim->config.required_compiles)
			return (pthread_mutex_unlock(&sim->coders[i].m), 0);
		pthread_mutex_unlock(&sim->coders[i].m);
	}
	return (1);
}

void	*monitor_routine(void *arg)
{
	t_sim	*sim;
	long	now;
	long	last_start;
	int		i;

	sim = (t_sim *)arg;
	while (!sim_should_stop(sim))
	{
		now = get_time_ms();
		i = -1;
		while (++i < sim->config.nb_of_coders)
		{
			pthread_mutex_lock(&sim->coders[i].m);
			last_start = sim->coders[i].last_compile_start_ms;
			pthread_mutex_unlock(&sim->coders[i].m);
			if (now - last_start > sim->config.time_to_burnout)
			{
				log_state(sim, sim->coders[i].id, "burned out");
				return (sim_set_stop(sim), sim_wake_all(sim), NULL);
			}
		}
		if (all_done(sim))
			return (sim_set_stop(sim), sim_wake_all(sim), NULL);
		usleep(1000);
	}
	return (NULL);
}
