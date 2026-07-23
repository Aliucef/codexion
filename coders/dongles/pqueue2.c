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

#include "pqueue.h" // t_dongle, t_waiter, pq_sift_up, pq_sift_down

// Inserts a new coder into the dongle's priority queue (min-heap).
// Always appends at the end, then bubbles up to restore heap order.
// Example: heap=[{id=1,prio=500}], push(coder_id=3, priority=742):
//   queue[1]={id=3,prio=742}, queue_size=2, sift_up(1):
//   parent=0 → {id=1,prio=500} wins → no swap → heap=[{1,500},{3,742}].
void	pq_push(t_dongle *d, int coder_id, long priority)
{
	int	i; // index where the new entry will land = current tail of the array

	i = d->queue_size;           // e.g. if 2 entries exist, new entry goes at index 2
	d->queue[i].coder_id = coder_id; // e.g. queue[2].coder_id = 3
	d->queue[i].priority = priority;  // e.g. queue[2].priority = 742
	d->queue_size++;             // bump size from 2 to 3
	pq_sift_up(d, i);           // bubble up so lowest priority stays at root
}

// Removes and returns the coder_id at the root (highest priority = lowest value).
// Replaces root with the last element, shrinks size, then sifts down.
// Example: heap=[{id=1,prio=500},{id=3,prio=742}], pop():
//   id=1 saved, root replaced with {id=3,prio=742}, queue_size=1, sift_down(0): no children → done.
//   Returns 1.
int	pq_pop(t_dongle *d)
{
	int	id; // the winner's coder_id, e.g. 1

	if (d->queue_size == 0) // safety check — should never happen in normal flow
		return (-1);        // empty heap — nothing to pop
	id = d->queue[0].coder_id;          // save the root's coder_id, e.g. 1
	d->queue_size--;                     // shrink the logical size, e.g. 2 → 1
	d->queue[0] = d->queue[d->queue_size]; // move last entry to root, e.g. queue[0]={id=3,prio=742}
	pq_sift_down(d, 0);                  // restore heap order from the new root downward
	return (id);                         // return the winner, e.g. 1
}

// Returns the coder_id at the heap root without removing it.
// This is the next coder that should be served (highest priority).
// Example: heap=[{id=1,prio=500},{id=3,prio=742}] → returns 1.
// Returns -1 if the queue is empty (no one waiting for this dongle).
int	pq_peek(t_dongle *d)
{
	if (d->queue_size == 0) // no waiters → signal nobody
		return (-1);        // e.g. after all coders have compiled and left
	return (d->queue[0].coder_id); // e.g. coder 1 (the most urgent waiter)
}

// Removes a specific coder from anywhere in the heap (not just the root).
// Used for cleanup when the sim stops mid-wait — the coder never took the dongle.
// Example: heap=[{id=1,prio=500},{id=3,prio=742},{id=5,prio=300}], remove(coder_id=3):
//   find i=1, replace queue[1] with last={id=5,prio=300}, queue_size=2,
//   sift_up(1): parent=0={id=1,prio=500}, prio 300 < 500 → swap → [{id=5,prio=300},{id=1,prio=500}].
void	pq_remove(t_dongle *d, int coder_id)
{
	int	i; // index of the entry we want to remove

	i = 0; // start searching from the root
	while (i < d->queue_size && d->queue[i].coder_id != coder_id)
		i++;                            // scan until we find coder_id, e.g. stops at i=1 for coder 3
	if (i == d->queue_size)            // coder not found — nothing to remove (already gone)
		return ;
	d->queue_size--;                   // shrink, e.g. 3 → 2
	d->queue[i] = d->queue[d->queue_size]; // overwrite the found slot with the last entry
	pq_sift_up(d, i);   // the replacement might be smaller than its parent → bubble up
	pq_sift_down(d, i); // or larger than its children → bubble down (only one direction will move)
}
