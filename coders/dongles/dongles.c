/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongles.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alyousse <alyousse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/18 09:20:58 by alyousse          #+#    #+#             */
/*   Updated: 2026/07/22 00:00:00 by alyousse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../parser/parse.h"
#include "../stop/stop.h"
#include "../logs/log.h"
#include "pqueue.h"
#include <time.h>

static long	get_priority(t_sim *sim, int coder_id)
{
	long	deadline;

	if (sim->config.scheduler == SCED_EDF)
	{
		pthread_mutex_lock(&sim->coders[coder_id - 1].m);
		deadline = sim->coders[coder_id - 1].last_compile_start_ms
			+ sim->config.time_to_burnout;
		pthread_mutex_unlock(&sim->coders[coder_id - 1].m);
		return (deadline);
	}
	return (get_time_ms());
}

static void	coder_wait(t_sim *sim, t_dongle *d, int coder_id)
{
	struct timespec	ts;
	pthread_cond_t	*cond;

	cond = &sim->coders[coder_id - 1].wait_cond;
	if (pq_peek(d) == coder_id && !d->held)
	{
		ts.tv_sec = d->cooldown_until_ms / 1000;
		ts.tv_nsec = (d->cooldown_until_ms % 1000) * 1000000L;
		pthread_cond_timedwait(cond, &d->mutex, &ts);
	}
	else
		pthread_cond_wait(cond, &d->mutex);
}

static int	dongle_wait_loop(t_sim *sim, t_dongle *d, int coder_id)
{
	long	now;

	while (!sim_should_stop(sim))
	{
		now = get_time_ms();
		if (pq_peek(d) == coder_id && !d->held && now >= d->cooldown_until_ms)
			return (1);
		coder_wait(sim, d, coder_id);
	}
	return (0);
}

int	dongle_take(t_sim *sim, int dongle_idx, int coder_id)
{
	t_dongle	*d;
	long		priority;

	d = &sim->dongles[dongle_idx];
	priority = get_priority(sim, coder_id);
	pthread_mutex_lock(&d->mutex);
	pq_push(d, coder_id, priority);
	if (!dongle_wait_loop(sim, d, coder_id))
	{
		pq_remove(d, coder_id);
		pthread_mutex_unlock(&d->mutex);
		return (0);
	}
	pq_pop(d);
	d->held = 1;
	pthread_mutex_unlock(&d->mutex);
	log_state(sim, coder_id, "has taken a dongle");
	return (1);
}

void	dongle_release(t_sim *sim, int dongle_idx)
{
	t_dongle	*d;
	int			next_id;

	d = &sim->dongles[dongle_idx];
	pthread_mutex_lock(&d->mutex);
	d->held = 0;
	d->cooldown_until_ms = get_time_ms() + sim->config.dongle_cooldown;
	next_id = pq_peek(d);
	if (next_id != -1)
		pthread_cond_signal(&sim->coders[next_id - 1].wait_cond);
	pthread_mutex_unlock(&d->mutex);
}
