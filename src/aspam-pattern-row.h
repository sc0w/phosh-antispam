/* aspam-pattern-row.h
 *
 * Copyright 2021 Chris Talbot
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once
#include <handy.h>
#include <gio/gio.h>

G_BEGIN_DECLS

#define ASPAM_TYPE_PATTERN_ROW (aspam_pattern_row_get_type ())

G_DECLARE_FINAL_TYPE (ASpamPatternRow, pattern_row, ASPAM, PATTERN_ROW, HdyActionRow)

ASpamPatternRow *aspam_pattern_row_new (void);

G_END_DECLS
