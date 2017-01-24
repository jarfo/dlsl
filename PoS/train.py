#-*- coding: utf-8 -*-

import numpy as np
import utils
from spanishGenWin import spanishGenWin as swg #Script for creating Gender Windows
from spanishNumWin import spanishNumWin as swn #Script for creating Number Windows 
from keras.models import Sequential
from keras.layers import Dense, Embedding, Flatten

#Path were the corpus is located
PATH = 'corpus/' 

#Size of the vocabulary to use
vocabSize = 5000

#Windows size
winSize = 5

VSIZE = 100
nb_epoch = 4
batch_size = 128

print "Processing corpus" 

#Change the function accordingly to the task (swg, swg)
training = swn(PATH + 'train/train.gennum.es', \
    PATH + 'train/train.es', \
    vocabSize,winSize)

dev = swn(PATH + 'dev/dev.gennum.es', \
    PATH + 'dev/dev.es', \
	vocabSize,winSize)

test = swn(PATH + 'test/test.gennum.es', \
    PATH + 'test/test.es', \
	vocabSize,winSize)

trainWindows, trainTargets = training.process()
devWindows, devTargets  = dev.process()
testWindows, testTargets  = test.process()

vocabulary = utils.getVocabulary(trainWindows,winSize,vocabSize)

trainFeatures = utils.vectorizeWindows(trainWindows,vocabulary)
devFeatures = utils.vectorizeWindows(devWindows,vocabulary)
testFeatures = utils.vectorizeWindows(testWindows,vocabulary)

trainTargets = np.asarray(trainTargets)
devTargets = np.asarray(devTargets)
testTargets = np.asarray(testTargets)

print "Finished processing"

model = Sequential()
# Number of embedding vectors = vocabSize + UNK + <s> + <e>
model.add(Embedding(vocabSize + 3, VSIZE, input_length=winSize, input_dtype='int32'))
model.add(Flatten())
model.add(Dense(512, activation='relu'))
model.add(Dense(trainTargets.shape[1], activation='softmax'))
model.summary()
model.compile(loss='categorical_crossentropy',
              optimizer='rmsprop',
              metrics=['accuracy'])

history = model.fit(trainFeatures, trainTargets,
                    batch_size=batch_size, nb_epoch=nb_epoch,
                    verbose=1, validation_data=(devFeatures, devTargets))
score = model.evaluate(testFeatures, testTargets, verbose=0)
print('Test score:', score[0])
print('Test accuracy:', score[1])
