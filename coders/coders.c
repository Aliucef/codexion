/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coders.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alyousse <alyousse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/16 13:54:27 by alyousse          #+#    #+#             */
/*   Updated: 2026/02/16 14:00:26 by alyousse         ###   ########.fr       */
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
	int v;

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
	}

	log_state(c->sim, c->id, "exiting");
	return (NULL);
}


void	*monitor_routine(void *arg)
{
	t_sim	*sim = (t_sim *)arg;
	long	start = get_time_ms();

	while (get_time_ms() - start < 2000)
		usleep(50 * 1000);

	sim_set_stop(sim);

	// wake sleepers later when we have condvar waits (not needed yet)
	return (NULL);
}
