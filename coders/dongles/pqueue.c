/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pqueue.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alyousse <alyousse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/22 00:00:00 by alyousse          #+#    #+#             */
/*   Updated: 2026/07/22 00:00:00 by alyousse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "pqueue.h"

static int	pq_wins(t_waiter a, t_waiter b)
{
	if (a.priority != b.priority)
		return (a.priority < b.priority);
	return (a.coder_id < b.coder_id);
}

static void	pq_swap(t_waiter *q, int i, int j)
{
	t_waiter	tmp;

	tmp = q[i];
	q[i] = q[j];
	q[j] = tmp;
}

void	pq_sift_up(t_dongle *d, int i)
{
	int	parent;

	while (i > 0)
	{
		parent = (i - 1) / 2;
		if (!pq_wins(d->queue[i], d->queue[parent]))
			break ;
		pq_swap(d->queue, i, parent);
		i = parent;
	}
}

void	pq_sift_down(t_dongle *d, int i)
{
	int	best;
	int	left;
	int	right;

	while (1)
	{
		best = i;
		left = 2 * i + 1;
		right = 2 * i + 2;
		if (left < d->queue_size && pq_wins(d->queue[left], d->queue[best]))
			best = left;
		if (right < d->queue_size && pq_wins(d->queue[right], d->queue[best]))
			best = right;
		if (best == i)
			break ;
		pq_swap(d->queue, i, best);
		i = best;
	}
}
