#!/usr/bin/env python

# This script searches for duplicate files in the data directory and replaces them with symlinks.
# It is useful when distributing the linux version of the game.

"""
 Copyright 2007 Justin Azoff

  License:
      GPL
"""

import os

BLOCKSIZE = 1024 * 8

class Files:
    def __init__(self):
        self._files = {}
        self._open_files = 0
        self._max_open_files = 512
    
    def _open(self, fn, pos=0):
        """Open or re-open a file"""
        if self._open_files == self._max_open_files:
            self._close_a_file()
        fh = open(fn)
        if pos:
            fh.seek(pos)
        self._files[fn] = {'file': fh, 'pos': pos}

        self._open_files += 1
        return fh

    def open(self, fn):
        if fn in self._files:
            f = self._files[fn]
            if f['file']:
                return f['file']
            pos = f['pos']
        else :
            pos = 0

        return self._open(fn, pos)

    def _close_a_file(self):
        for k,v in self._files.items():
            if v['file']:
                self.close(k)
                return

    def close(self, fn, remove=False):
        f = self._files[fn]
        fh = f['file']
        if fh and not fh.closed:
            pos = fh.tell()
            fh.close()
            self._open_files -= 1
        else :
            remove = True

        if remove or pos == 0:
            del self._files[fn]
        else :
            f['pos'] = pos
            f['file'] = None

class dupfinder:
    def __init__(self,dirs=None):
        self._files = {}
        self._inodes = set()
        self._file_handles = Files()
        self._dirs = []
        self._bytes_read = 0
        self._group = 1
        self._num_files = 0
        if dirs:
            self.add_dirs(dirs)

    def add_dirs(self, dirs):
        self._dirs.extend(dirs)

    def _add_file(self, fn):
        if os.path.islink(fn):
            return
        try :
            stat = os.stat(fn)
            size = stat.st_size
            #i really only need to cache the inodes for files which
            #have size collisions, but this way I only have to make
            #one pass and 1 stat call...
            tup = (stat.st_dev, stat.st_ino)
            if tup in self._inodes:
                return
            self._inodes.add(tup)
            self._files.setdefault(size,[]).append(fn)
        except OSError:
            return

    def _walk_dir(self, dir):
        for path, dirs, files in os.walk(dir):
            for f in files:
                fn = os.path.join(path, f)
                self._add_file(fn)
                self._num_files += 1

    def _build_flist(self):
        for dir in self._dirs:
            self._walk_dir(dir)
        chain = [len(v) for v in self._files.values()]
        if chain:
            long = max(chain)
        else :
            long = 0
        self._longest_chain = long

    def _clear_singles(self):
        """Delete files from the list that do not even have a 
           size collision"""
        singles = []
        for size, files in self._files.iteritems():
            if len(files) == 1:
                singles.append(size)
        for size in singles:
            del self._files[size]

        self._groups = len(self._files)

    def _get_fh(self, path):
        return self._file_handles.open(path)

    def _close_fh(self, path):
        #print "Closing", path
        self._file_handles.close(path, remove=True)

    def _identical(self, files, split=False):
        
        while 1:
            chunks = {}
            chains = []
            for p in files:
                f = self._get_fh(p)
                chunk = f.read(BLOCKSIZE)
                self._bytes_read += BLOCKSIZE
                chunks.setdefault(chunk,[]).append(p)
            
            #test for EOF on all files - means I found duplicate files
            if len(chunks) == 1 and '' in chunks:
                for fn in files:
                    self._close_fh(fn)
                if not split:
                    return [files]
                else :
                    return files

            for matches in chunks.values():
                if len(matches) != 1:
                    #matches - a list of filenames, are possibly duplicates
                    chains.append(matches)
                else :
                    #for a chunk that had no dupes, I can stop reading this file
                    fn = matches[0]
                    self._close_fh(fn)
                    files.remove(fn) 
            #say I have 4 files, and 1 == 3 and 2 == 4, but 1 != 2, I need to
            #re-call this function with [1,3] and [2,4] as files
            #otherwise, all the files were the same so far, and I can stay in this loop
            if len(chains) != 1:
                break

        same = []
        for chain in chains:
            same.append(self._identical(chain, split=True))

        return same

    def _is_dup(self, size, files):
        if size == 0:
            return [files]

        return self._identical(files)

    def find_dups(self):
        self._build_flist()
        self._clear_singles()

        dupes = []
        for size, files in self._files.iteritems():
            same = self._is_dup(size, files)
            if same:
                dupes.extend(same)
            #self._progress()
            
        assert self._file_handles._open_files == 0
        return dupes
        

    def _progress(self):
        sys.stderr.write("\rProgress %d/%d" % (self._group, self._groups))
        if self._group == self._groups:
            sys.stderr.write("\n")
        sys.stderr.flush()
        self._group += 1
        

    def print_stats(self):
        print >> sys.stderr, "%d total files" % self._num_files
        print >> sys.stderr, "%d size collisions, max of length %d" % (self._groups, self._longest_chain)

        print >> sys.stderr, "bytes read %d" % self._bytes_read
 
def make_ln( file_list ):
	if len(file_list) > 0:
		# TODO: use an appropriate python function, instead of calling ln
		main_file = file_list[0]	# make the first file the "master" file
		file_list = file_list[1:]	# all the files we'll replace with links
	 
		for del_f in file_list:
			os.system("ln -sf '" + main_file + "' '" + del_f +"'")
 
 
def convert(paths):
	# paths is a list of paths to convert duplicates to links
 
	d = dupfinder()
	d.add_dirs(paths)
 
	for dups in d.find_dups():
		make_ln( dups )
 
 
if __name__ == '__main__':
	import sys
 
	convert(sys.argv[1:])
	sys.exit(0)

