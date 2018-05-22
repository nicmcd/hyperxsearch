#!/usr/bin/env python3

import argparse

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
  dims = len(args.widths)
  terms = args.concentration
  routers = 1
  for width in args.widths:
    terms *= width
    routers *= width
  radix = args.concentration
  bisecs = [0] * len(args.widths)
  channels = [terms]
  for idx in range(len(args.widths)):
    radix += ((args.widths[idx] - 1) * args.weights[idx])
    bisecs[idx] = ((args.widths[idx] * args.weights[idx]) /
                   (2 * args.concentration))
    dimChannels = tri(args.widths[idx]) * args.weights[idx]
    for idx2 in range(len(args.widths)):
      if idx != idx2:
        dimChannels *= args.widths[idx2]
    channels.append(dimChannels)

  # get strings of everything
  dims_label = 'Dimensions'
  dims_value = str(dims)
  widths_label = 'Widths'
  widths_value = '[' + ','.join([str(w) for w in args.widths]) + ']'
  weights_label = 'Weights'
  weights_value = '[' + ','.join([str(w) for w in args.weights]) + ']'
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
  print('{0}{1} {2}{3} {4}{5} {6}{7} {8}{9} {10}{11} {12}{13} {14}{15} {16}{17}'
        .format(
          dims_label,
          gap(dims_label, dims_value),
          widths_label,
          gap(widths_label, widths_value),
          weights_label,
          gap(weights_label, weights_value),
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
  print('{0}{1} {2}{3} {4}{5} {6}{7} {8}{9} {10}{11} {12}{13} {14}{15} {16}{17}'
        .format(
          dims_value,
          gap(dims_value, dims_label),
          widths_value,
          gap(widths_value, widths_label),
          weights_value,
          gap(weights_value, weights_label),
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
  ap.add_argument('widths', type=str,
                  help='dimension widths')
  ap.add_argument('weights', type=str,
                  help='dimension weights')
  ap.add_argument('concentration', type=int,
                  help='concentration')
  ap.add_argument('-v', '--verbose', default=False, action='store_true',
                  help='turn on verbose output')
  args = ap.parse_args()

  args.widths = [int(x) for x in args.widths.split(',')]
  args.weights = [int(x) for x in args.weights.split(',')]

  for width in args.widths:
    assert width > 1
  for weight in args.weights:
    assert weight > 0
  assert len(args.widths) == len(args.weights)
  assert args.concentration > 0

  main(args)
