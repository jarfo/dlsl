# -*- coding: utf-8 -*-
'''An implementation of sequence to sequence learning for performing phonetic transcription
Padding is handled by using a repeated sentinel character (space)

Input may optionally be inverted, shown to increase performance in many tasks in:
"Learning to Execute"
http://arxiv.org/abs/1410.4615
and
"Sequence to Sequence Learning with Neural Networks"
http://papers.nips.cc/paper/5346-sequence-to-sequence-learning-with-neural-networks.pdf
Theoretically it introduces shorter term dependencies between source and target.

'''

from __future__ import print_function
from keras.utils.visualize_util import plot
from keras.models import Sequential
from keras.layers import Activation, TimeDistributed, Dense, RepeatVector, recurrent, Embedding, Reshape
import numpy as np
from six.moves import range

def levenshtein(s, t):
    """
        levenshtein(s, t) -> ldist
        ldist is the Levenshtein distance between the strings
        s and t.
        For all i and j, dist[i,j] will contain the Levenshtein
        distance between the first i characters of s and the
        first j characters of t
    """
    rows = len(s)+1
    cols = len(t)+1
    dist = np.zeros((rows, cols), dtype=np.int)
    # source prefixes can be transformed into empty strings
    # by deletions:
    for i in range(1, rows):
        dist[i,0] = i
    # target prefixes can be created from an empty source string
    # by inserting the characters
    for i in range(1, cols):
        dist[0,i] = i

    row = rows - 1
    col = cols - 1
    for col in range(1, cols):
        for row in range(1, rows):
            if s[row-1] == t[col-1]:
                cost = 0
            else:
                cost = 1
            dist[row, col] = min(dist[row-1, col] + 1,      # deletion
                                 dist[row, col-1] + 1,      # insertion
                                 dist[row-1, col-1] + cost) # substitution

    return dist[row, col]


class Dictionary(dict):
    def __init__(self, filename):
        with open(filename, 'rt') as f:
            for line in f:
                word, phones = line.split('\t')[:2]
                self.update({word: phones})
        super(Dictionary, self).__init__()


class CharacterTable(object):
    def __init__(self, chars, maxlen):
        self.chars = [''] + sorted(set(chars))
        self.indices = dict((c, i) for i, c in enumerate(self.chars))
        self.maxlen = maxlen
        self.size = len(self.chars)

    def encode(self, C, maxlen=None):
        maxlen = maxlen if maxlen else self.maxlen
        X = np.zeros(maxlen, dtype='int32')
        for i, c in enumerate(C):
            X[i] = self.indices[c]
        return X

    def decode(self, X, calc_argmax=False, ch=''):
        if calc_argmax:
            X = X.argmax(axis=-1)
        return ch.join(self.chars[x] for x in X)


class colors:
    ok = '\033[92m'
    fail = '\033[91m'
    close = '\033[0m'

# Parameters for the model and dataset
TRAIN='wcmudict.train.dict'
TEST='wcmudict.test.dict'

# Try replacing GRU, or SimpleRNN
RNN = recurrent.LSTM
VSIZE = 7
HIDDEN_SIZE = 128
BATCH_SIZE = 128
LAYERS = 1
INVERT = True

train = Dictionary(TRAIN)
test = Dictionary(TEST)

words = map(str.split, train.keys())    # [['u', 's', 'e', 'd'], ['m', 'o', 'u', 'n', 't'], ...]
trans = map(str.split, train.values())
words_maxlen = max(map(len, words))
trans_maxlen = max(map(len, trans))

chars = sorted(set([char for word in words for char in word]))
phones = sorted(set([phone for tran in trans for phone in tran]))
ctable = CharacterTable(chars, words_maxlen)
ptable = CharacterTable(phones, trans_maxlen)

print('Total training words:', len(words))

print('Vectorization...')
def vectorization(words, trans):
    X = np.zeros((len(words), words_maxlen), dtype='int32')
    y = np.zeros((len(trans), trans_maxlen), dtype='int32')
    for i, word in enumerate(words):
        X[i,:] = ctable.encode(word)
    for i, tran in enumerate(trans):
        y[i,:] = ptable.encode(tran)
    if INVERT:
        X = X[:,::-1]
    return X, y

X_train, y_train = vectorization(words, trans)

words_test = map(str.split, test.keys())
trans_test = map(str.split, test.values())
X_val, y_val = vectorization(words_test, trans_test)

# Shuffle (X_train, y_train) in unison
indices = np.arange(len(y_train))
np.random.shuffle(indices)
X_train = X_train[indices]
y_train = y_train[indices]

print(X_train.shape)
print(y_train.shape)

print('Build model...')
model = Sequential()
# "Encode" the input sequence using an RNN, producing an output of HIDDEN_SIZE
model.add(Embedding(ctable.size, VSIZE, input_dtype='int32'))
model.add(RNN(HIDDEN_SIZE))
# For the decoder's input, we repeat the encoded input for each time step
model.add(RepeatVector(trans_maxlen))
# The decoder RNN could be multiple layers stacked or a single layer
for _ in range(LAYERS):
    model.add(RNN(HIDDEN_SIZE, return_sequences=True))

# For each of step of the output sequence, decide which phone should be chosen
model.add(TimeDistributed(Dense(ptable.size)))
model.add(Activation('softmax'))
model.summary()
plot(model, show_shapes=True, to_file='pho_rnn.png', show_layer_names=False)

model.compile(loss='sparse_categorical_crossentropy',
              optimizer='adam',
              metrics=['accuracy'])

def save(refs, preds, filename):
    with open(filename, 'wt') as res:
        for ref, pred in zip(refs, preds):
            correct = ptable.decode(ref, ch=' ').strip()
            guess = ptable.decode(pred, calc_argmax=False, ch=' ').strip()
            print(correct, '|', guess, file=res)

# Train the model each generation and show predictions against the validation dataset
for iteration in range(1, 120):
    print()
    print('-' * 50)
    print('Iteration', iteration)
    model.fit(X_train, y_train[..., np.newaxis], batch_size=BATCH_SIZE, nb_epoch=1,
              validation_data=(X_val, y_val[..., np.newaxis])) # add a new dim to y_train and y_val to match output
    preds = model.predict_classes(X_val, verbose=0)
    save(y_val, preds, 'rnn_{}.pred'.format(iteration))

    ###
    # Select 10 samples from the validation set at random so we can visualize errors
    for i in range(10):
        ind = np.random.randint(0, len(X_val))
        rowX, rowy = X_val[ind], y_val[ind]
        pred = preds[ind]
        q = ctable.decode(rowX)
        correct = ptable.decode(rowy, ch=' ').strip()
        guess = ptable.decode(pred, ch=' ').strip()
        print('W:', q[::-1] if INVERT else q)
        print('T:', correct)
        print(colors.ok + '☑' + colors.close if correct == guess else colors.fail + '☒' + colors.close, 
              guess, '(' + str(levenshtein(correct.split(), guess.split())) + ')')
        print('---')
