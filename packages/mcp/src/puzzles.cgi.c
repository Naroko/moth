#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "common.h"


int
longcmp(long *a, long *b)
{
  if (*a < *b) return -1;
  if (*a > *b) return 1;
  return 0;
}

#define PUZZLES_MAX 100

/** Keeps track of the most points yet awarded in each category */
int    ncats = 0;
struct {
  char cat[CAT_MAX];
  long points;
} points_by_cat[PUZZLES_MAX];


size_t
read_until_char(FILE *f, char *buf, size_t buflen, char delim)
{
  size_t i = 0;

  while (1) {
    int c = fgetc(f);

    if ((EOF == c) || delim == c) {
      if (buf) {
        buf[i] = '\0';
      }
      return i;
    }

    if (i + 1 < buflen) {
      buf[i] = c;
    }
    i += 1;
  }
}

int
main(int argc, char *argv[])
{
  int i;
  DIR *srv;

  if (-1 == cgi_init(argv)) {
    return 0;
  }

  {
    FILE *f = fopen(state_path("puzzles.db"), "r");
    char  cat[CAT_MAX];
    char  points_str[11];
    long  points;
    int   i;

    while (f && (! feof(f))) {
      read_until_char(f, NULL, 0, ' ');
      read_until_char(f, cat, sizeof(cat), ' ');
      read_until_char(f, points_str, sizeof(points_str), '\n');
      points = atol(points_str);

      for (i = 0; i < ncats; i += 1) {
        if (0 == strcmp(cat, points_by_cat[i].cat)) break;
      }
      if (i == ncats) {
        if (PUZZLES_MAX == ncats) {
          continue;
        }
        strncpy(points_by_cat[i].cat, cat, sizeof(points_by_cat[i].cat));
        points_by_cat[i].points = 0;
        ncats += 1;
      }
      if (points > points_by_cat[i].points) {
        points_by_cat[i].points = points;
      }
    }

    if (f) fclose(f);
  }

  srv = opendir(package_path(""));
  if (NULL == srv) {
    cgi_error("Cannot opendir(\"/srv\")");
  }

  cgi_head("Open puzzles");
  printf("<dl>\n");

  /* For each file in /srv/ ... */
  while (1) {
    struct dirent *e          = readdir(srv);
    char          *cat;
    DIR           *puzzles;
    long           catpoints[PUZZLES_MAX];
    size_t         ncatpoints = 0;

    if (! e) break;

    cat = e->d_name;
    if ('.' == cat[0]) continue;
    /* We have to lstat anyway to see if it's a directory; may as
       well just barge ahead and watch for errors. */

    /* Open /srv/ctf/$cat/puzzles/ */
    puzzles = opendir(package_path("%s/puzzles", cat));
    if (NULL == puzzles) {
      continue;
    }

    while (ncatpoints < PUZZLES_MAX) {
      struct dirent *pe = readdir(puzzles);
      long           points;
      char          *p;

      if (! pe) break;

      /* Only do this if it's an int */
      points = strtol(pe->d_name, &p, 10);
      if (*p) continue;

      catpoints[ncatpoints++] = points;
    }

    closedir(puzzles);

    /* Sort points */
    qsort(catpoints, ncatpoints, sizeof(*catpoints),
          (int (*)(const void *, const void *))longcmp);


    /* Print out point values up to one past the last solved puzzle in
       this category */
    {
      long maxpoints = 0;

      /* Find the most points scored in this category */
      for (i = 0; i < ncats; i += 1) {
        if (0 == strcmp(cat, points_by_cat[i].cat)) {
          maxpoints = points_by_cat[i].points;
          break;
        }
      }

      printf("  <dt>%s</dt>\n", cat);
      printf("  <dd>\n");
      for (i = 0; i < ncatpoints; i += 1) {
        printf("    <a href=\"/%s/%ld\">%ld</a>\n",
               cat, catpoints[i], catpoints[i]);
        if (catpoints[i] > maxpoints) break;
      }
      printf("  </dd>\n");
    }
  }

  closedir(srv);

  printf("</dl>\n");
  cgi_foot();

  return 0;
}