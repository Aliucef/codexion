/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   stop.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alyousse <alyousse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/18 13:12:38 by alyousse          #+#    #+#             */
/*   Updated: 2026/02/18 13:12:38 by alyousse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "stop.h"
#include "../parser/parse.h"

int	sim_should_stop(t_sim *sim)
{
	int	val;

	pthread_mutex_lock(&sim->stop_m);
	val = sim->stop;
	pthread_mutex_unlock(&sim->stop_m);
	return (val);
}

void	sim_set_stop(t_sim *sim)
{
	pthread_mutex_lock(&sim->stop_m);
	sim->stop = 1;
	pthread_mutex_unlock(&sim->stop_m);
}
