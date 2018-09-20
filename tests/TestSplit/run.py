from pysys.constants import *
from apama.basetest import ApamaBaseTest
from apama.correlator import CorrelatorHelper
import re

class PySysTest(ApamaBaseTest):
	def execute(self):
		correlator = CorrelatorHelper(self, name='testcorrelator')
		correlator.start(logfile='testcorrelator.log', config=self.input+'/config.yaml')
		correlator.injectEPL(filenames=['ConnectivityPluginsControl.mon'], filedir=PROJECT.APAMA_HOME+'/monitors')
		correlator.injectEPL(filenames=['ConnectivityPlugins.mon'], filedir=PROJECT.APAMA_HOME+'/monitors')
		correlator.injectEPL(filenames=['test.mon'])
		correlator.flush() 
		self.waitForSignal(file='testcorrelator.log', expr='Round-trip', condition='>=200')

	def validate(self):
		self.assertGrep('testcorrelator.log', expr='ERROR', contains=False)
		self.assertLineCount('testcorrelator.log', expr='Round-trip OK', condition='==200')
		self.assertLineCount('before.txt', expr='Towards Host: {foo:[0-9]*,sag.type:Data,sag.channel:batchChain} / {s:Hello World}', condition='==100')
		self.assertLineCount('before.txt', expr='Towards Transport: {foo:[0-9]*,sag.type:Data,sag.channel:batchChain} / {s:Hello World}', condition='==100')
		for i in range(0, 11):
			self.assertGrep('after.txt', expr='Towards Transport: {foo:%s,sag.type:Data,sag.channel:batchChain} / \\[{s:Hello World}' % i, condition='>=1')
			self.assertGrep('after.txt', expr='Towards Host: {foo:%s,sag.type:Data,sag.channel:batchChain} / \\[{s:Hello World}' % i, condition='>=1')

