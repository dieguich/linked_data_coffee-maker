require(forecast)

# ===========================
# START Analysis
# ===========================
# Read the csv file
cpdata = read.csv("coffees_23.csv",header=F)
# Create the time series with frequency 12 (7am-7pm)
# We're going to create a learning time of 23 days so 23*12 = 276 (276 lines of in the csv)
cpd = ts(cpdata[1:276,1],start=1,freq=12)
summary(cpd)

# We continue using the Arima (3,1,1)(1,0,2)
fit <- Arima(cpd, order=c(3,1,1), seasonal=c(1,0,2))
summary(fit)

# We perform the prediction for the next 5 days (Mon-Fry) 12*5-1 = 59
frcast <- forecast(fit,h=60)
summary(frcast)
# write in a new csv the prediction with the confidence intervals
write.csv(frcast, "forecast.csv", row.names = FALSE)