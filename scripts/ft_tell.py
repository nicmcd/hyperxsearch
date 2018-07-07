#!/usr/bin/env python3

import argparse
from functools import reduce
from operator import mul

def tri(width):
  return (width * (width - 1)) // 2

def gap(used, other):
  spacing = max(len(used), len(other)) - len(used)
  assert spacing >= 0
  return ' ' * spacing

def main(args):
  if args.verbose:
    print(args)

  # compute all info
  levels = len(args.ups)
  terms = 1
  routers_at_level = 1

  radices = []
  terms_per_group = []
  routers_at_level_per_group = [routers_at_level]
  total_groups = []

  for level in range(levels):
    down = args.downs[level]
    up = args.ups[level]
    radices.append((down, up, down + up))
    terms *= down
    terms_per_group.append(terms)
    if level < (levels - 1):
      routers_at_level *= up
      routers_at_level_per_group.append(routers_at_level)

  routers_at_level = []
  channels_at_level = []
  bisec_at_level = []
  for level in range(levels):
    groups = terms // terms_per_group[level]
    total_groups.append(groups)
    routers_at_level.append(groups * routers_at_level_per_group[level])
    channels_at_level.append(args.downs[level] * routers_at_level[level])
    if level == 0:
      bisec_at_level.append(float('inf'))
    else:
      thisBi = ((args.downs[level] * routers_at_level_per_group[level]) /
                terms_per_group[level])
      bisec_at_level.append(min(min(bisec_at_level), thisBi))

  """
  print('level,radices,terms_per_group,routers_at_level_per_group,total_groups,'
        'routers_at_level,channels_at_level,bisec_at_level')
  for level in range(levels):
    print('{},{}-{}-{},{},{},{},{},{},{}'.format(
      level, radices[level][0], radices[level][1], radices[level][2],
      terms_per_group[level], routers_at_level_per_group[level],
      total_groups[level], routers_at_level[level], channels_at_level[level],
      bisec_at_level[level]))
  """

  routers = sum(routers_at_level)
  channels = sum(channels_at_level)

  # get strings of everything
  levels_label = 'Levels'
  levels_value = str(levels)
  ports_label = 'Ports'
  ports_value = args.down_ups
  terms_label = 'Terminals'
  terms_value = str(terms)
  routers_label = 'Routers'
  routers_value = '[' + ','.join([str(r) for r in routers_at_level]) + ']'
  total_routers_label = 'TotalRouters'
  total_routers_value = str(routers)
  radix_label = 'Radix'
  radix_value = '[' + ','.join([str(r[2]) for r in radices]) + ']'
  channels_label = 'Channels'
  channels_value = '[' + ','.join([str(c) for c in channels_at_level]) + ']'
  total_channels_label = 'TotalChannels'
  total_channels_value = str(channels)
  bisec_label = 'Bisection'
  bisec_value = '[' + ','.join(['{:.02f}%'.format(b * 100)
                                for b in bisec_at_level[1:]]) + ']'

  # print info
  print('{0}{1} {2}{3} {4}{5} {6}{7} {8}{9} {10}{11} {12}{13} {14}{15} {16}{17}'
        .format(
          levels_label,
          gap(levels_label, levels_value),
          ports_label,
          gap(ports_label, ports_value),
          terms_label,
          gap(terms_label, terms_value),
          routers_label,
          gap(routers_label, routers_value),
          total_routers_label,
          gap(total_routers_label, total_routers_value),
          radix_label,
          gap(radix_label, radix_value),
          channels_label,
          gap(channels_label, channels_value),
          total_channels_label,
          gap(total_channels_label, total_channels_value),
          bisec_label,
          gap(bisec_label, bisec_value)))
  print('{0}{1} {2}{3} {4}{5} {6}{7} {8}{9} {10}{11} {12}{13} {14}{15} {16}{17}'
        .format(
          levels_value,
          gap(levels_value, levels_label),
          ports_value,
          gap(ports_value, ports_label),
          terms_value,
          gap(terms_value, terms_label),
          routers_value,
          gap(routers_value, routers_label),
          total_routers_value,
          gap(total_routers_value, total_routers_label),
          radix_value,
          gap(radix_value, radix_label),
          channels_value,
          gap(channels_value, channels_label),
          total_channels_value,
          gap(total_channels_value, total_channels_label),
          bisec_value,
          gap(bisec_value, bisec_label)))

if __name__ == '__main__':
  ap = argparse.ArgumentParser()
  ap.add_argument('down_ups', type=str,
                  help='down and up ports (e.g., 8x4-6x3-10)')
  ap.add_argument('-v', '--verbose', default=False, action='store_true',
                  help='turn on verbose output')
  args = ap.parse_args()

  levels = [x.split('x') for x in args.down_ups.split('-')]
  args.downs = []
  args.ups = []
  for idx, level in enumerate(levels):
    for ud in level:
      assert int(ud) > 0, 'only values greater than 0 are valid'
    if idx == len(levels) - 1:
      assert len(level) == 1, 'the last level must be a single value'
      args.ups.append(0)
    else:
      assert len(level) == 2, 'all non-last levels must have two values'
      args.ups.append(int(level[1]))
    args.downs.append(int(level[0]))

  main(args)
