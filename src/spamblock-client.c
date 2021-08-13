/* spamblock-client.c
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

#include "spamblock-client.h"

struct _SpamBlock
{
  GObject     parent_instance;
};

G_DEFINE_TYPE (SpamBlock, spam_block, G_TYPE_OBJECT)

static void
spam_block_constructed (GObject *object)
{
  //SpamBlock *self = (SpamBlock *)object;

  G_OBJECT_CLASS (spam_block_parent_class)->constructed (object);

}

static void
spam_block_finalize (GObject *object)
{
  //SpamBlock *self = (SpamBlock *)object;

  G_OBJECT_CLASS (spam_block_parent_class)->finalize (object);
}

static void
spam_block_class_init (SpamBlockClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->constructed = spam_block_constructed;
  object_class->finalize = spam_block_finalize;
}

static void
spam_block_init (SpamBlock *self)
{
}

SpamBlock *
spam_block_get_default (void)
{
  static SpamBlock *self;

  if (!self)
    {
      self = g_object_new (VVM_TYPE_VVMD, NULL);
      g_object_add_weak_pointer (G_OBJECT (self), (gpointer *)&self);
    }
  return self;
}
