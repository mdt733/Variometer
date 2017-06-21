struct table_1d {
  uint8_t x_length;
  long *x_values;
  long *y_values;
};

float interpolate_segment(float x0, float y0, float x1, float y1, float x)
{
  float t;
  if (x <= x0) return y0;
  if (x >= x1) return y1;
  t =  (x - x0);
  t /= (x1 - x0);
  return y0 + t * (y1 - y0);
}

float interpolate_table_1d(struct table_1d * table, float x) //TODO: interpolate search
{
  uint8_t segment;

  /* Check input bounds and saturate if out-of-bounds */
  if (x > (table->x_values[table->x_length - 1])) return table->y_values[table->x_length - 1];
  else if (x < (table->x_values[0])) return table->y_values[0];

  /* Find the segment that holds x */
  for (segment = 0; segment < (table->x_length - 1); segment++)
  {
    if ((table->x_values[segment]   <= x) &&
        (table->x_values[segment + 1] >= x))
    {
      return interpolate_segment(table->x_values[segment],   /* x0 */
                                 table->y_values[segment],   /* y0 */
                                 table->x_values[segment + 1], /* x1 */
                                 table->y_values[segment + 1], /* y1 */
                                 x);                         /* x  */
    }
  }
  return table->y_values[table->x_length - 1];
}
