#-*- coding: utf-8 -*-
import numpy as np
import operator

def vectorizeWindows(windows,vocabulary):
	vectorizedWindows = []
	for i, group in enumerate(windows):
		indexes = []
		for g in group:
			if g not in ( '<s>', '<e>'):
				try:
					indexes.append(vocabulary.index(g)+1)
				except ValueError:
					#Token desconocido
					indexes.append(len(vocabulary)+1)	
			else:
				if g == '<s>':
					indexes.append(0)
				else:
					indexes.append(len(vocabulary)+2)	
		vectorizedWindows.append(indexes)		 
	return vectorizedWindows

'''
def vectorizeSentences(features,corpus):
	vectorizedSentences = []
	for i, group in enumerate(features):
		indexes = []
		for g in group:
			if g != u'':
				indexes.append(corpus.index(g)+1)
		vectorizedSentences.append(indexes)		 
	return vectorizedSentences


def makeClasses(targets):
	classes = np.empty((0,2), dtype=theano.config.floatX)	
	for i in targets:
		act = np.array([1.0,0.0]) if i == 0 else np.array([0.0,1.0])
		classes = np.vstack([classes,act])
	return classes	
'''

def getVocabulary(windows,winSize=7,vocabSize=7000):
	vocabulary = {}
	for w in windows:
		for l in w:
			if vocabulary.has_key(l):
				vocabulary[l] += 1
			else:
				vocabulary[l] = 1

	pairs = vocabulary.items()
	pairs = sorted(pairs, key=lambda x: x[1], reverse=True) 
	orderedVocabulary = [x[0] for x in pairs]
	orderedVocabulary = orderedVocabulary[:vocabSize]  \
	if vocabSize < len(orderedVocabulary) and vocabSize != 0 \
	else orderedVocabulary					

	return orderedVocabulary

	
