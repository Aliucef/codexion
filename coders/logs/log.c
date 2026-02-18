/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   log.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alyousse <alyousse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/18 13:12:15 by alyousse          #+#    #+#             */
/*   Updated: 2026/02/18 13:12:15 by alyousse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdio.h>
#include "../parser/parse.h"

void	log_state(t_sim *sim, int coder_id, const char *message)
{
	long	now;
	long	timestamp;

	if (!sim || !message)
		return ;
	now = get_time_ms();
	timestamp = now - sim->start_ms;
	pthread_mutex_lock(&sim->log_m);
	printf("%ld %d %s\n", timestamp, coder_id, message);
	pthread_mutex_unlock(&sim->log_m);
}
