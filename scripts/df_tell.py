#!/usr/bin/env python3

import argparse
import math

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
  terms = args.concentration * args.local_width * args.global_width
  routers = args.local_width * args.global_width
  radix = (args.concentration + ((args.local_width - 1) * args.local_weight) +
           math.ceil(((args.global_width - 1) * args.global_weight) /
                     args.local_width))
  bisecs = [((args.global_width * args.global_weight) /
             (2 * args.concentration * args.local_width)),
            ((args.local_width * args.local_weight) /
             (2 * args.concentration))]

  channels = [terms,
              args.global_width * tri(args.local_width) * args.local_weight,
              tri(args.global_width) * args.global_weight]

  # get strings of everything
  gwidth_label = 'GlobalWidth'
  gwidth_value = str(args.global_width)
  gweight_label = 'GlobalWeight'
  gweight_value = str(args.global_weight)
  lwidth_label = 'LocalWidth'
  lwidth_value = str(args.local_width)
  lweight_label = 'LocalWeight'
  lweight_value = str(args.local_weight)
  conc_label = 'Concentration'
  conc_value = str(args.concentration)
  terms_label = 'Terminals'
  terms_value = str(terms)
  routers_label = 'Routers'
  routers_value = str(routers)
  radix_label = 'Radix'
  radix_value = str(radix)
  channels_label = 'Channels'
  channels_value = '[' + ','.join([str(c) for c in channels]) + ']'
  bisec_label = 'Bisections'
  bisec_value = '[' + ','.join(['{0:.5f}'.format(b) for b in bisecs]) + ']'

  # print info
  print('{0}{1} {2}{3} {4}{5} {6}{7} {8}{9} {10}{11} {12}{13} {14}{15} {16}{17} {18}{19}'
        .format(
          gwidth_label,
          gap(gwidth_label, gwidth_value),
          gweight_label,
          gap(gweight_label, gweight_value),
          lwidth_label,
          gap(lwidth_label, lwidth_value),
          lweight_label,
          gap(lweight_label, lweight_value),
          conc_label,
          gap(conc_label, conc_value),
          terms_label,
          gap(terms_label, terms_value),
          routers_label,
          gap(routers_label, routers_value),
          radix_label,
          gap(radix_label, radix_value),
          channels_label,
          gap(channels_label, channels_value),
          bisec_label,
          gap(bisec_label, bisec_value)))
  print('{0}{1} {2}{3} {4}{5} {6}{7} {8}{9} {10}{11} {12}{13} {14}{15} {16}{17} {18}{19}'
        .format(
          gwidth_value,
          gap(gwidth_value, gwidth_label),
          gweight_value,
          gap(gweight_value, gweight_label),
          lwidth_value,
          gap(lwidth_value, lwidth_label),
          lweight_value,
          gap(lweight_value, lweight_label),
          conc_value,
          gap(conc_value, conc_label),
          terms_value,
          gap(terms_value, terms_label),
          routers_value,
          gap(routers_value, routers_label),
          radix_value,
          gap(radix_value, radix_label),
          channels_value,
          gap(channels_value, channels_label),
          bisec_value,
          gap(bisec_value, bisec_label)))

if __name__ == '__main__':
  ap = argparse.ArgumentParser()
  ap.add_argument('global_width', type=int,
                  help='number of groups')
  ap.add_argument('global_weight', type=int,
                  help='number of links between each pair of groups')
  ap.add_argument('local_width', type=int,
                  help='number of routers per group')
  ap.add_argument('local_weight', type=int,
                  help='number of links between each pair of routers in groups')
  ap.add_argument('concentration', type=int,
                  help='concentration')
  ap.add_argument('-v', '--verbose', default=False, action='store_true',
                  help='turn on verbose output')
  args = ap.parse_args()

  assert args.global_width > 1
  assert args.global_weight >= 1
  assert args.local_width > 1
  assert args.local_weight >= 1
  assert args.concentration > 0

  main(args)
