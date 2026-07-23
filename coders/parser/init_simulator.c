/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init_simulator.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alyousse <alyousse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/18 12:42:43 by alyousse          #+#    #+#             */
/*   Updated: 2026/07/23 09:33:07 by alyousse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdio.h>
#include <stdlib.h>
#include "parse.h"
#include <string.h>
#include "../validation/validate.h"
#include <sys/time.h>

static int	sim_init_cleanup(t_sim *sim, int coders_inited, int dongles_inited)
{
	int	k;

	k = 0;
	while (k < dongles_inited)
	{
		free(sim->dongles[k].queue);
		pthread_mutex_destroy(&sim->dongles[k].mutex);
		k++;
	}
	free(sim->dongles);
	sim->dongles = NULL;
	k = -1;
	while (++k < coders_inited)
	{
		pthread_cond_destroy(&sim->coders[k].wait_cond);
		pthread_mutex_destroy(&sim->coders[k].m);
	}
	free(sim->coders);
	sim->coders = NULL;
	pthread_mutex_destroy(&sim->log_m);
	pthread_mutex_destroy(&sim->stop_m);
	return (0);
}

static int	init_coders(t_sim *sim, int *out_inited)
{
	int		i;
	t_coder	*c; // coders details

	*out_inited = 0;
	sim->coders = malloc(sizeof(t_coder) * sim->config.nb_of_coders); // allocating coders with the number of coders
	if (!sim->coders) // fail
		return (0);
	i = 0;
	while (i < sim->config.nb_of_coders) //loop on each coder
	{
		c = &sim->coders[i]; // assign each coder
		memset(c, 0, sizeof(*c)); // allocate it with 0 independently
		c->id = i + 1; // id them in sequence
		c->sim = sim; // hmmmmmm
		c->last_compile_start_ms = sim->start_ms; // hmmmmm
		if (pthread_mutex_init(&c->m, NULL) != 0)
			return (0);
		if (pthread_cond_init(&c->wait_cond, NULL) != 0)
			return (pthread_mutex_destroy(&c->m), 0);
		(*out_inited)++;
		i++;
	}
	return (1);
}

static int	init_dongles(t_sim *sim, int *out_inited)
{
	int			i;
	t_dongle	*d;

	*out_inited = 0;
	sim->dongles = malloc(sizeof(t_dongle) * sim->config.nb_of_coders);
	if (!sim->dongles)
		return (0);
	i = -1;
	while (++i < sim->config.nb_of_coders)
	{
		d = &sim->dongles[i];
		memset(d, 0, sizeof(*d));
		d->cooldown_until_ms = sim->start_ms;
		d->queue = malloc(sizeof(t_waiter) * sim->config.nb_of_coders);
		if (!d->queue)
			return (0);
		if (pthread_mutex_init(&d->mutex, NULL) != 0)
			return (free(d->queue), d->queue = NULL, 0);
		(*out_inited)++;
	}
	return (1);
}

static int	init_mutexes(t_sim *sim)
{
	if (pthread_mutex_init(&sim->stop_m, NULL) != 0) // initializing a mutex
		return (0); // on fail = 0 , quit
	if (pthread_mutex_init(&sim->log_m, NULL) != 0) // initializing again
		return (pthread_mutex_destroy(&sim->stop_m), 0); // if we get to here it means the first mutex has been initialized but the second failed , so we need to distroy the first
	return (1); // success
}

int	init_sim(t_sim *sim, const t_parse *config) // config is args
{
	int	coders_inited; //number of coders
	int	dongles_inited; //number of dongles

	if (!sim || !config || config->nb_of_coders <= 0) // on any failure or negative coders number
		return (0);
	memset(sim, 0, sizeof(*sim)); // allocat sim with 0 size of sim
	sim->config = *config; // set the args to be in sim
	sim->start_ms = get_time_ms(); // set start time , why? idk yet
	sim->stop = 0; // stop to false
	if (!init_mutexes(sim)) // initialize mutex
		return (0); // quit
	if (!init_coders(sim, &coders_inited)) // initialize each coder
		return (sim_init_cleanup(sim, coders_inited, 0)); // clean up on fail
	if (!init_dongles(sim, &dongles_inited)) // initialize each dongle
		return (sim_init_cleanup(sim, coders_inited, dongles_inited)); // clean up on fail
	return (1);
}
