#library(plotly)		# libreria per plottare

computeExperimentValues = function(path, conf_int = FALSE)
{
	# media di timeDetectionNeg, timeDetectionNeg e F-measure tra tutti i nodi di tutti i Run
	
	file = read.csv(path)
	
	N = 20
	
	col1 = 1	# F-measure
	col2 = 8	# timeDetectionNeg:mean
	col3 = 15	# timeDetectionPos:mean
	col4 = 4	# precision
	col5 = 5	# recall
	
	for(j in 1:nrow(file))
	{
		if(file[1,j] == "F-measure")
			col1 = j
		else if(file[1,j] == "timeDetectionNeg:stats:mean")
			col2 = j
		else if(file[1,j] == "timeDetectionPos:stats:mean")
			col3 = j
			
	}
	
	dataTimeNeg = rbind()
	dataTimePos = rbind()
	dataFmeasure = rbind()
	dataPrecision = rbind()
	dataRecall = rbind()
	
	# ciclo sui valori (per riga) ignorando i NaN
	for(j in 2:ncol(file))		# parte dalla colonna 2 perché la 1 è il nome del campo
	{
		# F-measure
		if(is.numeric(file[col1,j]) && ! is.nan(file[col1,j]))
			dataFmeasure = rbind(dataFmeasure, file[col1,j])
			
		# timeDetectionNeg
		if(is.numeric(file[col2,j]) && ! is.nan(file[col2,j]))
			dataTimeNeg = rbind(dataTimeNeg, file[col2,j])
		
		# timeDetectionPos
		if(is.numeric(file[col3,j]) && ! is.nan(file[col3,j]))
			dataTimePos = rbind(dataTimePos, file[col3,j])
			
		if(is.numeric(file[col4,j]) && ! is.nan(file[col4,j]))
			dataPrecision = rbind(dataPrecision, file[col4,j])
		
		if(is.numeric(file[col5,j]) && ! is.nan(file[col5,j]))
			dataRecall = rbind(dataRecall, file[col5,j])
	}

	meanFmeasure = mean(dataFmeasure)
	varFmeasure = var(dataFmeasure)	
	errorFmeasure = qt(0.95, df=N-1)*varFmeasure/sqrt(N)
	#print(paste("F-measure - stima della media:",meanFmeasure," - varianza della media:", varFmeasure," - intervallo di confidenza (95):",meanFmeasure-error,meanFmeasure+error))

	meanTimeNeg = mean(dataTimeNeg)
	varTimeNeg = var(dataTimeNeg)
	errorTimeNeg = qt(0.95, df=N-1)*varTimeNeg/sqrt(N)
	#print(paste("F-measure - stima della media:",meanTimeNeg," - varianza della media:", varTimeNeg," - intervallo di confidenza (95):",meanTimeNeg-error,meanTimeNeg+error))

	meanTimePos = mean(dataTimePos)
	varTimePos = var(dataTimePos)
	errorTimePos = qt(0.95, df=N-1)*varTimePos/sqrt(N)
	#print(paste("F-measure - stima della media:",meanTimePos," - varianza della media:", varTimePos," - intervallo di confidenza (95%):",meanTimePos-error,meanTimePos+error))
	
	meanPrecision = mean(dataPrecision)
	meanRecall = mean(dataRecall)
	
	if(conf_int)
		# restituisce gli intervalli di confidenza
		# restituisce gli intervalli di confidenza
		return(c(
			c(meanFmeasure-errorFmeasure, meanFmeasure+errorFmeasure),
			c(meanTimeNeg-errorTimeNeg, meanTimeNeg+errorTimeNeg),
			c(meanTimePos-errorTimePos, meanTimePos+errorTimePos)
		))
	else
		return(c(meanFmeasure, meanTimeNeg, meanTimePos, meanPrecision, meanRecall))
}



# funzione generica per disegnare un grafico 3d; è chiamata dal quelle sotto
createGraph = function(x, y, z, n, path, baseDirName, baseFileName, ext, labelX, labelY)
{
	for(j in y)
	{
		for(i in x)
		{
			filePath = paste(path, baseDirName, i, baseFileName, j, ext, sep = "")
			ret = computeExperimentValues(filePath)
			
			z = c(z ,ret[n])
		}
	}
	
	z = matrix(z, nrow = length(x), ncol = length(y))		# modifico da lista a matrix
	
	# scelta del colore in base alla tipologia
	if(n == 1)
	{
		color = "green"
		label = "F-measure"
	}
	else if (n == 2)
	{
		color = "lightblue"
		label = "timeDetectionNeg (s)"
	}
	else if(n == 3)
	{
		color = "red"
		label = "timeDetectionPos (s)"
	}
	else if(n == 4)
	{
		color = "yellow"
		label = "Precision"
	}
	else if(n == 5)
	{
		color = "darkolivegreen3"
		label = "Recall"
	}
	else
	{
		color = "gray"
		label = ""
	}
	
	# mostra grafico 3d
	# xlab, ylab e zlab sono le etichette dei 3 assi cartesiani; theta e phi sono gli angoli di rotazione; expand scala rispetto a z
	persp(x,y,z, xlab= labelX, ylab= labelY, zlab= label, theta = 45, phi = 35, col = color, ticktype='detailed', expand = 0.5)
		
	print(z)	
}

# prende i valori da i file csv di ogni experimento e costruisce un grafico 3d
createGraphIncrease = function(n, path = '/home/mattia/Documents/Projects/omnetpp/ACDC/DATA/increase')
{
	baseDirName = '/threshold-'
	baseFileName = '/Scalar-DelayLimit-'
	ext = '-1.csv'
	
	x = c(0.06, 0.08, 0.1)		# valori per threshold
	y = c(0.3, 0.4, 0.6, 0.8)	# valori per delayLimit
	z = c()
	
	createGraph(x, y, z, n, path, baseDirName, baseFileName, ext, "threshold", "delayLimit")
}


# Caso correlation: prende i valori da i file csv di ogni experimento e costruisce un grafico 3d
createGraphCorrelation = function(n, path = '/home/mattia/Documents/Projects/omnetpp/ACDC/DATA/correlation')
{
	baseDirName = '/repetitions-'
	baseFileName = '/Scalar-minCorrelation-'
	ext = '-1.csv'
	
	x = c(10, 14, 18)		# valori per threshold
	y = c(0.52, 0.57, 0.62, 0.68)	# valori per delayLimit
	z = c()
	
	createGraph(x, y, z, n, path, baseDirName, baseFileName, ext, "repetitions", "minCorrelation")
}


# Caso correlation: prende i valori da i file csv di ogni experimento e costruisce un grafico 3d
createGraphCorrelation3 = function(n, path = '/home/mattia/Documents/Projects/omnetpp/ACDC/DATA/correlation3')
{
	baseDirName = '/minCorr-'
	baseFileName = '/Scalar-nChampions-'
	ext = '-1.csv'
	
	x = c(0.57, 0.62, 0.68)		# valori per threshold
	y = c(8, 12, 16, 20)	# valori per delayLimit
	z = c()
	
	createGraph(x, y, z, n, path, baseDirName, baseFileName, ext, "minCorrelation", "nChampions")
}


#getVariance(exp, val1, val2)
#{
#	col1 = 11
#	col2 = 18
#	
#	if(exp == "increase")
#	{
#		path = '/home/mattia/Documents/Projects/omnetpp/ACDC/DATA/increase')
#		baseDirName = '/threshold-'
#		baseFileName = '/Scalar-DelayLimit-'
#		ext = '-1.csv'
#		
#		filePath = paste(path, baseDirName, i, baseFileName, j, ext, sep = "")
#		file = read.csv(filePath)
#		
#		
#	}
#}
