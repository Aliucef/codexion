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


void	*coder_routine(void *arg)
{
	t_coder	*c = (t_coder *)arg;

	while (!sim_should_stop(c->sim))
	{
		log_state(c->sim, c->id, "is alive");
		usleep(200 * 1000); // 200ms, just for testing
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
