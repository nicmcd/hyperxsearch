#!/usr/bin/env python3

import argparse
import functools
import operator

def tri(width):
  return (width * (width - 1)) // 2

def gap(used, other):
  spacing = max(len(used), len(other)) - len(used)
  assert spacing >= 0
  return ' ' * spacing

def main(args):
  if args.verbose:
    print(args)

  # compute some info
  dims = len(args.widths)
  terms = args.concentration
  for width in args.widths:
    terms *= width

  # check
  assert args.terminal < terms
  assert abs(args.offset) < terms

  # apply the offset
  args.terminal = (args.terminal + args.offset) % terms

  # compute coordinate
  sizes = [args.concentration]
  sizes.extend(args.widths)

  rem = args.terminal
  coord = [-1] * (dims + 1)
  assert len(coord) == len(sizes)
  for idx in range(len(sizes)):
    coord[idx] = rem % sizes[idx]
    rem //= sizes[idx]

  print('terminal {} is '.format(args.terminal), end='')
  if args.bigendian:
    print('{}-{}'.format(','.join([str(x) for x in reversed(coord[1:])]), coord[0]))
  else:
    print('{}-{}'.format(coord[0], ','.join([str(x) for x in coord[1:]])))


if __name__ == '__main__':
  ap = argparse.ArgumentParser()
  ap.add_argument('widths', type=str,
                  help='dimension widths')
  ap.add_argument('concentration', type=int,
                  help='concentration')
  ap.add_argument('terminal', type=int,
                  help='terminal')
  ap.add_argument('-o', '--offset', type=int, default=0,
                  help='a cyclic offset from the terminal')
  ap.add_argument('-b', '--bigendian', action='store_true',
                  help='use big endian coordinate format')
  ap.add_argument('-v', '--verbose', default=False, action='store_true',
                  help='turn on verbose output')
  args = ap.parse_args()

  args.widths = [int(x) for x in args.widths.split(',')]

  for width in args.widths:
    assert width > 1
  assert args.concentration > 0

  main(args)
