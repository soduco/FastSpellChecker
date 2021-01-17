from ._backend import CPPDictionary

class Dictionary:

    def __init__(self,
                 file_or_wordlist = None,
                 normalize_fn = None,
                 max_distance = 2):
        '''
        Create a new dictionary.

        Args
        ====

        :param file_or_wordlist (str): A list of strings or an opened file containing the words to insert
        :param normalize_fn (str): Function used to normalize words (e.g. lowercase conversion...)
        :max_distance (int): The maximum distance allowed when searching for candidates
        '''

        if normalize_fn:
            self.normalize = normalize_fn
        self._max_distance = max_distance

        self.load(file_or_wordlist)


    def load(self, file_or_wordlist):
        '''
        Initialize (or reset) the dictionary from a list of words or a file containing words (1 per line)

        Args
        ====

        :param file_or_wordlist (str): A list of strings or an opened file containing the words to insert
        '''
        self._impl = CPPDictionary()
        if file_or_wordlist is not None:
            for word in file_or_wordlist:
                word = word.rstrip()
                word = self.normalize(word)
                self._impl.add_word(word)

    def best_match(self, word: str, d = -1):
        '''
        Return a best match for a given word (limited to a given distance).

        Args
        ====

        :param word (str): The word to search
        :param d (int): Maximum errors


        :return: A dictionary with fields:
                 - word: (one of) the closest match in the dictionary
                 - distance: The distance with the closest match
                 - count: The number of words matching with this distance in the dictionary
        '''
        if d > self._max_distance:
            raise ValueError("Distance ({}) exceeds the max distance capacity (){})".format(d, self._max_distance))
        if len(word) > self._impl.max_word_length():
            raise ValueError("The size (={}) of the string exceeds the maximim word length (={}).".format(len(word), self._impl.max_word_length()))

        word = self.normalize(word)
        d = d if d >= 0 else self._max_distance
        return self._impl.best_match(word, d)


    def has_matches(self, word: str, d = -1):
        if d > self._max_distance:
            raise ValueError("Distance ({}) exceeds the max distance capacity (){})".format(d, self._max_distance))
        if len(word) > self._impl.max_word_length():
            raise ValueError("The size (={}) of the string exceeds the maximim word length (={}).".format(len(word), self._impl.max_word_length()))

        word = self.normalize(word)
        d = d if d >= 0 else self._max_distance
        return self._impl.has_matches(word, d)

    def candidates(self, word: str, d = -1):
        '''
        Return the list of candidate words in the dictionary at a given distances
        '''
        raise NotImplementedError()

    def __contains__(self, word: str):
        '''
        Check if a word belongs to the dictionary
        '''
        return self.has_matches(word, 0)


    @staticmethod
    def normalize(word):
        return word
