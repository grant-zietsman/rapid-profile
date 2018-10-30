#!/usr/bin/python3

class Statistic:

    def __init__(self):
        self.min = None
        self.max = None
        self.sum = 0
        self.cnt = 0

    def clear(self):
        self.min = None
        self.max = None
        self.sum = 0
        self.cnt = 0

    def sample(self, value):
        self.min = min(self.min, value) if self.min is not None else value
        self.max = max(self.max, value) if self.max is not None else value
        self.cnt += 1
        self.sum += value

    def results(self):
        return [
            self.cnt,
            self.min,
            self.max,
            self.sum / self.cnt if self.cnt > 0 else None
        ]


class Tag:

    def __init__(self):
        self.name = ""
        self.file = ""
        self.line = 0

def csv(file, delimeter=','):
    return [line.strip().split(delimeter) for line in file]

def get_statistic(id):
    statistic = Statistic()
    durations = []
    with open('intervals.rp.csv', 'r') as fintervals:
        fintervals.readline()
        for row in csv(fintervals):
            if id == row[0]:
                duration = float(row[3])
                statistic.sample(duration)
                durations.append(duration)            
    return durations, statistic

def get_statistics():
    statistics = {}
    with open('intervals.rp.csv', 'r') as fintervals:
        fintervals.readline()
        for row in csv(fintervals):
            id = row[0]
            if id not in statistics:
                statistics[id] = Statistic()

            duration = float(row[3])
            statistic = statistics[id]
            statistic.sample(duration)            
    return statistics

def get_tags():
    tags = {}
    with open('tags.rp.csv', 'r') as ftags:
        ftags.readline()
        for row in csv(ftags):
            id = row[0]
            if id not in tags:
                tags[id] = Tag()

            tags[id].name = row[1]
            tags[id].file = row[2]
            tags[id].line = int(row[3])
    return tags

def overview():
    statistics = get_statistics()
    tags = get_tags()
    print("-" * 85)
    print("%5s %32s %10s %10s %10s %10s" % ("id", "name", "cnt", "min", "max", "avg"))
    print("-" * 85)
    for id in sorted(statistics):
        tag = tags[id]
        stats = statistics[id].results()
        print("%5s %32s %10u %10.2f %10.2f %10.2f" % (id, tag.name, *stats[0:4]))
    print("-" * 85)

def interval(id):
    durations, statistic = get_statistic(str(id))
    tags = get_tags()
    tag = tags[str(id)]
    stats = statistic.results()
    print("-" * 85)
    print("%5s %32s %10s %10s %10s %10s" % ("id", "name", "cnt", "min", "max", "avg"))
    print("-" * 85)
    print("%5s %32s %10u %10.2f %10.2f %10.2f" % (id, tag.name, *stats[0:4]))
    print("-" * 85)
    print()
    print("%s" % ("iterations"))
    print()
    for duration in durations:
        print("%.2f" % (duration))
    print()


try:
    import argparse

    parser = argparse.ArgumentParser(description='Rapid profile analyzer.')
    parser.add_argument('--id', '-i', type=int, help='interval ID to query', default=None)
    args = parser.parse_args()

    if args.id is None:
        overview()
    else:
        interval(args.id)
except:
    print('FALLBACK MODE\n')
    import sys
    if (len(sys.argv) > 1):
        key = sys.argv[1]
        if key == '-i':
            if len(sys.argv) > 2:
                id = sys.argv[2]
                interval(id)
            else:
                print('-i required ID argument')
        else:
            try:
                id = sys.argv[1]
                interval(id)
            except:
                print('unrecognized argument argument')            
    else:
        overview()


