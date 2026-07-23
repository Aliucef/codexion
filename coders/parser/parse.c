/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alyousse <alyousse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/09 09:15:39 by alyousse          #+#    #+#             */
/*   Updated: 2026/02/18 12:42:24 by alyousse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdio.h>                   // printf() (used by callers for error output)
#include <stdlib.h>                  // atoi()
#include "parse.h"                   // t_parse, t_sched, SCED_FIFO, SCED_EDF
#include <string.h>                  // (included for completeness; not directly used here)
#include "../validation/validate.h"  // parse_scheduler()
#include <sys/time.h>                // struct timeval, gettimeofday()

// Converts all 8 command-line arguments into the t_parse config struct.
// argv[1..7] are numeric (milliseconds), argv[8] is the scheduler name.
// Example: argv = {"./codexion","5","800","200","200","200","3","50","fifo"}
void	init_arguments(t_parse *args, char **argv)
{
	args->nb_of_coders = atoi(argv[1]);       // e.g. "5"    → nb_of_coders = 5
	args->time_to_burnout = atoi(argv[2]);    // e.g. "800"  → time_to_burnout = 800ms
	args->time_to_compile = atoi(argv[3]);    // e.g. "200"  → time_to_compile = 200ms
	args->time_to_debug = atoi(argv[4]);      // e.g. "200"  → time_to_debug = 200ms
	args->time_to_refactor = atoi(argv[5]);   // e.g. "200"  → time_to_refactor = 200ms
	args->required_compiles = atoi(argv[6]);  // e.g. "3"    → required_compiles = 3
	args->dongle_cooldown = atoi(argv[7]);    // e.g. "50"   → dongle_cooldown = 50ms
	if (!parse_scheduler(argv[8], &args->scheduler)) // e.g. "fifo" → args->scheduler = SCED_FIFO
		return ; // invalid scheduler string — is_valid() already caught this, so this is a safeguard
}

// Returns the current wall-clock time in milliseconds.
// Used as a shared timestamp by all threads (burnout checks, FIFO priority, EDF deadline).
// Example: if gettimeofday returns {tv_sec=1753224000, tv_usec=742000}
//   → return 1753224000 * 1000 + 742000 / 1000 = 1753224000742ms.
long	get_time_ms(void)
{
	struct timeval	tv; // holds seconds and microseconds from the OS

	gettimeofday(&tv, NULL);                    // fill tv with current time (NULL = no timezone)
	return (tv.tv_sec * 1000L + tv.tv_usec / 1000L); // convert to ms: sec→ms + μs→ms
}
