#-*- coding: utf-8 -*-
from window import window
#from windowAll import window

class spanishNumWin(window):

	def __init__(self,textEs,textTg, vocabSize=7000, winSize = 7):
		super(spanishNumWin,self).__init__(textEs,textTg, vocabSize, winSize)
		self.dictPos = {'V':5, 'N':3,'A':4,'S':4,'D':4,'P':4}
		self.dictTg = {'S':1, 'P':0}

	def getTarget(self, word):
		target = [0.0,0.0,0.0]
		if '|' not in word:
			target[0] = -1
			return target
		word, base, POS = word.split('|')
		if self.dictPos.has_key(POS[0]):
			try:
				index = self.dictPos[POS[0]]
				index = self.dictTg[POS[index]]
				target[index] = 1.0
			except KeyError:
				target[2] = 1.0
		else:
			target[0] = -1
		return target				
	
