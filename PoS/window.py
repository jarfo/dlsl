#-*- coding: utf-8 -*-

import numpy as np


def read_lines(file):
	while True:
		line = file.readline()
		if not line:
			break
		yield line	

class window(object):
	
	def __init__(self,textEs,textTg, vocabSize=7000, winSize = 7):
		self.textEs = textEs
		self.textTg = textTg
		self.vocabSize = vocabSize
		self.winSize = winSize		

			
	def process(self,makeTargets=True):
		fileTextEs = open(self.textEs)
		fileTargetsEs = open(self.textTg)

		targets = []
		windows = []
		words = []
		vocabulary = {}

		for line in read_lines(fileTextEs):
			targetLine = next(read_lines(fileTargetsEs))
			winLine, tLine = self.line2windows(line,targetLine,makeTargets)
			windows += winLine
			targets += tLine
		fileTextEs.close()
		fileTargetsEs.close()

		return 	windows,targets
			
		
	def line2windows(self,line,targetline,makeTargets):
		line = line.replace('\n','').split()
		targetline = targetline.replace('\n','').split()
		windows = []
		targets = []
		w = self.winSize/2

		for i,l in enumerate(line):
			targetI = [-1,0,0]

			if makeTargets:
				targetI = self.getTarget(targetline[i])
				if targetI[0] != -1 and '[' in l:
					targets.append(targetI)					

			if targetI[0] != -1 or not makeTargets:
				if l[0] in ['D','A','V','N','S','P'] and '[' in l:
					
					#Fill the parts of the window with '<s>' for the words previous to the start of the sentence
					#'<e>' for the words after the end of the sentence  					
					wprev = w if w <= i else i
					wpos = w if (w + i) <= len(line)-1 else len(line)-i-1
					win = ('<s> '*(w-wprev)).split() + \
						line[i-wprev:i] + [line[i]] + \
						line[i+1:i+wpos+1] + ('<e> '*(w-wpos)).split()
					
					windows.append(win)

		return windows, targets


	




