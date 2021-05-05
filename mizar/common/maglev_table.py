import logging
from zlib import crc32, adler32

logging.basicConfig()
logger = logging.getLogger()
logger.setLevel(logging.INFO)


class MaglevTable:

    def __init__(self, table_size=101):
        # Some good prime numbers: 101, 211, 307, 401, 503, 601, 701, 809, 907, 1009, 2003
        # at least 100x greater than size and prime.
        self.table_size = table_size
        self.permutations_table = {}
        # total unique elements
        self.size = len(self.permutations_table.keys())
        self.table = []
        self.prev_table = []
        self.prev_elements_map = {}
        self.elements_replaced = 0
        self.elements_replacing = 0
        self.altered = False

    def get_table(self):
        """
        Returns the lookup table.
        The table must be repopulated if a change has been made.
        """
        if self.altered:
            self.prev_table = self.table
            self._populate_table()
            if self.prev_table:
                self._generate_prev_elements_map()
        if not self.table:
            raise Exception("Table is empty.")
        self.altered = False
        return self.table

    def add(self, element):
        """
        Adds an element to the permutation table, and computes its permutation array.
        """
        if element in self.permutations_table.keys():
            raise Exception("Element already exist.")

        permutation = self._compute_permutations(element)
        self.permutations_table[element] = permutation
        self.altered = True

    def remove(self, element):
        """
        Removes an element from the permutation table.
        """
        if element not in self.permutations_table.keys():
            raise Exception("Element not found.")
        self.permutations_table.pop(element)
        self.altered = True

    def get_size(self):
        return len(self.permutations_table.keys())

    def get_table_size(self):
        return self.table_size

    def get_prev_elements_map(self):
        """
        Returns a table mapping each unique
        element in the new table to elements
        it replaced in the previous table.
        """
        return self.prev_elements_map

    def _generate_prev_elements_map(self):
        """
        Generates a table mapping each unique
        element in the new table to elements
        it replaced in the previous table.
        """
        if not self.prev_table:
            raise Exception("Previous table is empty.")
        prev_elements_map = {}
        elements_replaced = 0
        for i in range(0, self.table_size):
            prev = self.prev_table[i]
            curr = self.table[i]
            if curr not in prev_elements_map.keys():  # Initialize
                prev_elements_map[curr] = []
            if curr != prev and prev not in prev_elements_map[curr]:
                prev_elements_map[curr].append(prev)
        for ele in list(prev_elements_map):
            elements_replaced += len(prev_elements_map[ele])
            if not prev_elements_map[ele]:  # Remove empty list mappings
                prev_elements_map.pop(ele)
        self.prev_elements_map = prev_elements_map
        self.elements_replaced = elements_replaced
        self.elements_replacing = len(prev_elements_map.keys())

    def _populate_table(self):
        """
        Populates the lookup table based upon the current permuations table.
        """
        next_ind = [0] * self.get_size()
        table = [None] * self.table_size
        n = 0
        while True:
            i = 0
            for element in sorted(self.permutations_table.keys()):
                cand = self.permutations_table[element][next_ind[i]]
                while table[cand]:
                    next_ind[i] += 1
                    cand = self.permutations_table[element][next_ind[i]]
                table[cand] = element
                next_ind[i] += 1
                n += 1
                if n == self.table_size:
                    self.table = table
                    return
                i += 1

    def _compute_permutations(self, element):
        """
        Computes the permutation array for the given element
        """
        permutations = [None] * self.table_size
        for i in range(0, self.table_size):
            offset = (crc32(element.encode("ascii"))
                      & 0xffffffff) % self.table_size
            skip = (adler32(element.encode("ascii")) &
                    0xffffffff) % (self.table_size - 1) + 1
            index = (offset + i * skip) % self.table_size
            permutations[i] = index
        return permutations

    def dump_table(self):
        logger.info("Act Lookup table: {}".format(self.table))

    def dump_permutations_table(self):
        for key in sorted(self.permutations_table.keys()):
            logger.info("Element name: {}, Permutations {}".format(
                key, self.permutations_table[key]))
