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

#include "pqueue.h" // t_dongle, t_waiter, and forward declarations for pq_sift_up/down

// Returns 1 if waiter 'a' should be served before waiter 'b'.
// Lower priority value = higher urgency (min-heap).
// Tie-break: lower coder_id wins (deterministic, required by EDF spec v1.5).
// Example FIFO: a={id=3, prio=742}, b={id=1, prio=800} → 742 < 800 → a wins.
// Example EDF tie: a={id=3, prio=1300}, b={id=1, prio=1300} → same prio → id 1 wins.
static int	pq_wins(t_waiter a, t_waiter b)
{
	if (a.priority != b.priority)    // e.g. 742 != 800 → compare by priority
		return (a.priority < b.priority); // e.g. 742 < 800 → return 1 (a wins)
	return (0); // equal priority → no preference (undefined tie-break)
}

// Swaps two entries in the heap array.
// Example: heap=[{id=5,prio=900},{id=3,prio=742}], swap(q,0,1)
// → heap=[{id=3,prio=742},{id=5,prio=900}]
static void	pq_swap(t_waiter *q, int i, int j)
{
	t_waiter	tmp; // temporary holder during the swap

	tmp = q[i];  // save entry at position i, e.g. {id=5, prio=900}
	q[i] = q[j]; // overwrite i with j, e.g. {id=3, prio=742}
	q[j] = tmp;  // put saved value at j, e.g. {id=5, prio=900}
}

// Bubbles entry at index 'i' upward until the heap property is restored.
// Called after inserting a new element at the end (pq_push puts it at index queue_size-1).
// Example: heap=[{id=1,prio=500}], insert {id=3,prio=742} at index 1.
//   parent of 1 = (1-1)/2 = 0 → {id=1,prio=500} wins over {id=3,prio=742} → stop.
// Example: heap=[{id=3,prio=742}], insert {id=5,prio=300} at index 1.
//   parent of 1 = 0 → prio 300 < 742 → swap → {id=5,prio=300} is now at root.
void	pq_sift_up(t_dongle *d, int i)
{
	int	parent; // index of parent node, formula: (i-1)/2

	while (i > 0)                           // stop when we reach the root (index 0)
	{
		parent = (i - 1) / 2;              // e.g. i=3 → parent=(3-1)/2=1
		if (!pq_wins(d->queue[i], d->queue[parent])) // child does NOT beat parent → heap ok
			break ;                        // stop bubbling up
		pq_swap(d->queue, i, parent);      // child beats parent → swap them
		i = parent;                        // continue checking from the parent's position
	}
}

// Bubbles entry at index 'i' downward until the heap property is restored.
// Called after removing the root (pq_pop replaces root with the last element).
// Example: heap=[{id=5,prio=900},{id=3,prio=742},{id=1,prio=500}] after pop:
//   root replaced with {id=1,prio=500}? No — last element placed at root:
//   heap=[{id=1,prio=500},{id=3,prio=742}], sift_down(0):
//   left child 1 = {id=3,prio=742} wins over root → swap → [{id=3,prio=742},{id=1,prio=500}].
void	pq_sift_down(t_dongle *d, int i)
{
	int	best;  // index of the winner among i, left, right
	int	left;  // index of left child: 2*i+1
	int	right; // index of right child: 2*i+2

	while (1) // keep going until no swap is needed
	{
		best = i;              // assume current node is best
		left = 2 * i + 1;     // e.g. i=0 → left=1
		right = 2 * i + 2;    // e.g. i=0 → right=2
		if (left < d->queue_size && pq_wins(d->queue[left], d->queue[best]))
			best = left;       // left child beats current best → update best
		if (right < d->queue_size && pq_wins(d->queue[right], d->queue[best]))
			best = right;      // right child beats left and current → update best
		if (best == i)         // no child beat us → heap property satisfied
			break ;            // done
		pq_swap(d->queue, i, best); // swap with the winning child
		i = best;                   // continue from where the winner now sits
	}
}
