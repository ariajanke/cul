#!/usr/bin/python3
import os
import pathlib
import datetime
import time
import platform
import re

class FileWatch:
  def __init__(self, filepath: str) -> None:
    self.path = pathlib.Path(filepath)
    self.filepath = filepath
    self.oldTimeStamp = self._modtime()

  def checkRefresh(self, f):
    newTimeStamp = self._modtime()
    if newTimeStamp != self.oldTimeStamp:
      f(self.filepath)
    self.oldTimeStamp = newTimeStamp

  def _modtime(self):
    return self.path.stat().st_mtime

def printOnUpdate(fn):
  print(f"{fn} has been updated!")

def main():
  watchers = []
  for root, dirs, filenames in os.walk('unit-tests'):
    for filename in filenames:
      if re.match(r'.+-tests\.cpp$', filename) == None:
        continue
      print(f"Watching file {root}/{filename}")
      watchers.append(FileWatch(f"{root}/{filename}"))

  if len(watchers) == 0:
    print('No files to watch!')
    return

  while True:
    for watcher in watchers:
      watcher.checkRefresh(printOnUpdate)
    time.sleep(2)

if __name__ == "__main__":
  main()