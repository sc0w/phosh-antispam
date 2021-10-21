/* aspam-pattern-row-client.c
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

#include "aspam-pattern-row.h"

struct _ASpamPatternRow
{
  HdyActionRow parent_instance;
};

G_DEFINE_TYPE (ASpamPatternRow, aspam_pattern_row, HDY_TYPE_ACTION_ROW)

static void
delete_button_clicked_cb (ASpamPatternRow *self)
{
  //ASpamSettings *settings;
  const char *pattern;
  //settings = aspam_settings_get_default ();
  //TODO: delete this new pattern

  pattern = hdy_action_row_get_subtitle (HDY_ACTION_ROW (self));
  g_debug ("Deleting %s", pattern);
  gtk_widget_destroy (GTK_WIDGET (self));
}

static void
aspam_pattern_row_constructed (GObject *object)
{
  //ASpamPatternRow *self = (ASpamPatternRow *)object;

  G_OBJECT_CLASS (aspam_pattern_row_parent_class)->constructed (object);

}

static void
aspam_pattern_row_finalize (GObject *object)
{
  //ASpamPatternRow *self = (ASpamPatternRow *)object;

  G_OBJECT_CLASS (aspam_pattern_row_parent_class)->finalize (object);
}

static void
aspam_pattern_row_class_init (ASpamPatternRowClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->constructed = aspam_pattern_row_constructed;
  object_class->finalize = aspam_pattern_row_finalize;

  gtk_widget_class_set_template_from_resource (widget_class,
                                             "/org/kop316/antispam/"
                                             "ui/aspam-pattern-row.ui");

  //gtk_widget_class_bind_template_child (widget_class, ASpamPatternRow, new_whitelist_button);

  gtk_widget_class_bind_template_callback (widget_class, delete_button_clicked_cb);

}

static void
aspam_pattern_row_init (ASpamPatternRow *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}

ASpamPatternRow *
aspam_pattern_row_new (void)
{
  return g_object_new (ASPAM_TYPE_PATTERN_ROW, NULL);
}

