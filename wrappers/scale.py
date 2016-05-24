#!/usr/bin/env python3

import argparse
import os
import subprocess
import taskrun
import threading


def getInfo(exe, maxradix, minterminals, minbandwidth, maxdimensions):
  cmd = ('{0} --maxradix {1} --minterminals {2} --minbandwidth {3} '
         '--maxdimensions {4} --maxresults 1').format(
           exe, maxradix, minterminals, minbandwidth, maxdimensions)
  stdout = subprocess.check_output(cmd, shell=True).decode('utf-8')
  lines = stdout.split('\n')
  if len(lines) < 3:
    return None
  else:
    best = lines[1].split()
    routers = float(best[6])
    channels = float(best[8])
    return routers, channels


lock = threading.Lock()
results = {}

def findLargestNetwork(exe, maxradix, minbandwidth, maxdimensions, verbose):
  if verbose:
    print('exe={0} maxradix={1} minbandwidth={2} maxdimensions={3}'
          .format(exe, maxradix, minbandwidth, maxdimensions))

  terms = None
  routers = None
  channels = None

  bot = 2
  top = maxradix**(args.maxdimensions+1)

  # verify top is unachievable
  info = getInfo(args.hyperxsearch, maxradix, top, args.minbandwidth,
                 args.maxdimensions)
  assert info is None, 'The programmer is an idiot!'

  # use a binary search to find the largest network possible
  while True:
    assert bot <= top
    mid = ((top - bot) // 2) + bot
    info = getInfo(args.hyperxsearch, maxradix, mid, args.minbandwidth,
                   args.maxdimensions)

    if verbose:
      print('bot={0} top={1} mid={2} solution={3}'.format(
        bot, top, mid, False if info is None else True))

    if bot == mid:
      assert info is not None
      terms = mid
      routers, channels = info
      break

    if info is None:
      top = mid - 1
    else:
      bot = mid

  if verbose:
    print('{0},{1},{2},{3},{4},{5}'.format(maxradix, terms, routers, channels,
                                           terms / routers, channels / terms))
  lock.acquire()
  results[maxradix] = (maxradix, terms, routers, channels,
                       terms / routers, channels / terms)
  lock.release()


def main(args):
  if args.verbose and args.cores > 1:
    print('verbose mode forces cores to 1')
    args.cores = 1
  if args.verbose:
    print('using {0} cores for execution'.format(args.cores))

  rm = taskrun.ResourceManager(
    taskrun.CounterResource('cores', 1, args.cores))
  tm = taskrun.TaskManager(resource_manager=rm, observer=None)

  for radix in range(args.minradix, args.maxradix+1, 1):
    task = taskrun.FunctionTask(
      tm, 'radix_{0}'.format(radix), findLargestNetwork, args.hyperxsearch,
      radix, args.minbandwidth, args.maxdimensions, args.verbose)
    task.priority = radix

  tm.run_tasks()

  print('radix,terms,routers,channels,terms/router,channels/term')
  for radix in range(args.minradix, args.maxradix+1, 1):
    print('{0},{1},{2},{3},{4},{5}'.format(*results[radix]))


if __name__ == '__main__':
  ap = argparse.ArgumentParser()
  ap.add_argument('hyperxsearch',
                  help='hyperxsearch executable')
  ap.add_argument('minradix', type=int,
                  help='minimum radix to search')
  ap.add_argument('maxradix', type=int,
                  help='maximum radix to search')
  ap.add_argument('maxdimensions', type=int,
                  help='maximum number of dimensions')
  ap.add_argument('minbandwidth', type=float,
                  help='minimum bisection bandwidth')
  ap.add_argument('-c', '--cores', type=int, default=os.cpu_count(),
                  help='number of cores to use')
  ap.add_argument('-v', '--verbose', default=False, action='store_true',
                  help='turn on verbose output')
  args = ap.parse_args()

  assert args.minradix <= args.maxradix

  main(args)
