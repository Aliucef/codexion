/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pqueue2.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alyousse <alyousse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/22 00:00:00 by alyousse          #+#    #+#             */
/*   Updated: 2026/07/22 00:00:00 by alyousse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "pqueue.h"

void	pq_push(t_dongle *d, int coder_id, long priority)
{
	int	i;

	i = d->queue_size;
	d->queue[i].coder_id = coder_id;
	d->queue[i].priority = priority;
	d->queue_size++;
	pq_sift_up(d, i);
}

int	pq_pop(t_dongle *d)
{
	int	id;

	if (d->queue_size == 0)
		return (-1);
	id = d->queue[0].coder_id;
	d->queue_size--;
	d->queue[0] = d->queue[d->queue_size];
	pq_sift_down(d, 0);
	return (id);
}

int	pq_peek(t_dongle *d)
{
	if (d->queue_size == 0)
		return (-1);
	return (d->queue[0].coder_id);
}

void	pq_remove(t_dongle *d, int coder_id)
{
	int	i;

	i = 0;
	while (i < d->queue_size && d->queue[i].coder_id != coder_id)
		i++;
	if (i == d->queue_size)
		return ;
	d->queue_size--;
	d->queue[i] = d->queue[d->queue_size];
	pq_sift_up(d, i);
	pq_sift_down(d, i);
}
