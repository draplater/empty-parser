from collections import namedtuple


class EdgeToEmpty(namedtuple("_", "head position")): # type: (int, int, int)
    pass
