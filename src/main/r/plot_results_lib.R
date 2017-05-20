#############################################################################
#
# RESULTS AND TIMINGS FROM STRIDE SIMULATIONS
#
#############################################################################

plot_results <- function(data_tag){


  ######################
  ## GET DATA       ##
  ######################

  data	   <- read.table(paste(data_tag,'_summary.csv',sep=''),header=TRUE,sep=",",stringsAsFactors=F)
  data_log <-read.table(paste(data_tag,'_cases.csv',sep=''),header=FALSE,sep=",")

  if(dim(data_log)[2]>1)
  {

  ######################
  ## START PDF STREAM	##
  ######################
  pdf(file=paste(data_tag,'_results.pdf'))


  ######################
  ## PLOT OVER TIME  	##
  ######################

  num_days <- dim(data_log)[2]
  plot(c(1,num_days),c(min(data_log),max(data_log)),col=0,xlab='time (days)',ylab='cases',main='TOTAL CASES')
  for(i in 1:dim(data_log)[1])
  {
    lines(as.double(data_log[i,]))
  }

  daily_cases <- data_log[,2:(num_days)] - data_log[,1:(num_days-1)]
  plot(c(1,num_days),c(min(daily_cases),max(daily_cases)),col=0,xlab='time (days)',ylab='daily cases',main='DAILY CASES')
  for(i in 1:dim(daily_cases)[1])
  {
    lines(as.double(daily_cases[i,]))
  }

  attack_rate <- daily_cases / data$pop_size
  plot(c(1,num_days),c(min(attack_rate),max(attack_rate)),col=0,xlab='time (days)',ylab='attack rate',main='ATTACK RATE')
  for(i in 1:dim(attack_rate)[1])
  {
    lines(as.double(attack_rate[i,]))
  }

  ######################
  ## PLOT TIMINGS     ##
  ######################

  plot(data$AR, data$total_time,xlab='attack rate',ylab='total time (s)',main='TIMINGS')

  ######################
  ## SINGLE PLOTS     ##
  ######################

  # input
  opt_seeding_rate  <- sort(unique(data$seeding_rate))
  opt_R0            <- sort(unique(data$R0))
  opt_immunity_rate <- sort(unique(data$immunity_rate))

  plot_AR_lim       <- c(0,1.2)
  plot_time_lim     <- c(0,max(data$total_time)*1.2)

  if(length(opt_seeding_rate)>1)
  {
    plot_pch <- as.factor(data$R0)
    plot_col <- as.factor(data$immunity_rate)

    plot(data$seeding_rate,data$AR,ylim=plot_AR_lim,main='SEED INFECTED: AR',xlab='seeding rate',ylab='attack rate',pch=as.numeric(plot_pch),col=as.numeric(plot_col))
    if(length(opt_seeding_rate)>1)  {legend('topleft',paste('R0:',levels(plot_pch)),pch=1:length(levels(plot_pch)))              }
    if(length(opt_immunity_rate)>1) {legend('topright',paste('immunity rate:',levels(plot_col)),fill=1:length(opt_immunity_rate)) }

    plot(data$seeding_rate,data$total_time,ylim=plot_time_lim,main='SEED INFECTED: TIME',xlab='seeding rate',ylab='total time (s)',pch=as.numeric(plot_pch),col=as.numeric(plot_col))
    if(length(opt_seeding_rate)>1)  {legend('topleft',paste('R0:',levels(plot_pch)),pch=1:length(levels(plot_pch)))              }
    if(length(opt_immunity_rate)>1) {legend('topright',paste('immunity rate:',levels(plot_col)),fill=1:length(opt_immunity_rate)) }
  }

  if(length(opt_R0)>1)
  {
    plot_pch <- as.factor(data$seeding_rate)
    plot_col <- as.factor(data$immunity_rate)

    plot(data$R0,data$AR,ylim=plot_AR_lim,main='R0',xlab='R0: AR',ylab='attack rate',pch=as.numeric(plot_pch),col=as.numeric(plot_col))
    if(length(opt_seeding_rate)>1)  {legend('topleft',paste('seeding rate:',levels(plot_pch)),pch=1:length(levels(plot_pch)))    }
    if(length(opt_immunity_rate)>1) {legend('topright',paste('immunity rate:',levels(plot_col)),fill=1:length(levels(plot_col))) }

    plot(data$R0,data$total_time,ylim=plot_time_lim,main='R0: TIME',xlab='R0',ylab='total time (s)',pch=as.numeric(plot_pch),col=as.numeric(plot_col))
    if(length(opt_seeding_rate)>1)  {legend('topleft',paste('seeding rate:',levels(plot_pch)),pch=1:length(levels(plot_pch)))    }
    if(length(opt_immunity_rate)>1) {legend('topright',paste('immunity rate:',levels(plot_col)),fill=1:length(opt_immunity_rate)) }
  }

  if(length(opt_immunity_rate)>1)
  {
    plot_pch <- as.factor(data$R0)
    plot_col <- as.factor(data$seeding_rate)

    plot(data$immunity_rate,data$AR,ylim=plot_AR_lim,main='POPULATION IMMUNITY: AR',xlab='immunity rate',ylab='attack rate',pch=as.numeric(plot_pch),col=as.numeric(plot_col))
    if(length(opt_R0)>1)  {legend('topleft',paste('R0:',levels(plot_pch)),pch=1:length(levels(plot_pch)))                      }
    if(length(opt_seeding_rate)>1) {legend('topright',paste('seeding rate:',levels(plot_col)),fill=1:length(opt_seeding_rate)) }

    # attack rate in susceptible population
    plot(data$immunity_rate,data$AR/(1-data$immunity_rate),ylim=plot_AR_lim,main='POPULATION IMMUNITY: AR (2)',xlab='immunity rate',ylab='attack rate in susceptibles',pch=as.numeric(plot_pch),col=as.numeric(plot_col))
    if(length(opt_R0)>1)  {legend('topleft',paste('R0:',levels(plot_pch)),pch=1:length(levels(plot_pch)))                      }
    if(length(opt_seeding_rate)>1) {legend('topright',paste('seeding rate:',levels(plot_col)),fill=1:length(opt_seeding_rate)) }

    #time
    plot(data$immunity_rate,data$total_time,ylim=plot_time_lim,main='POPULATION IMMUNITY: TIME',xlab='immunity rate',ylab='total time (s)',pch=as.numeric(plot_pch),col=as.numeric(plot_col))
    if(length(opt_R0)>1)  {legend('topleft',paste('R0:',levels(plot_pch)),pch=1:length(levels(plot_pch)))                      }
    if(length(opt_seeding_rate)>1) {legend('topright',paste('seeding rate:',levels(plot_col)),fill=1:length(opt_seeding_rate)) }
  }




  ######################
  ## CLOSE PDF STREAM ##
  ######################
  dev.off()

  } # end if(data_log)[2]>1)

} # end function
