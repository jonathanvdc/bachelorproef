#############################################################################
#  This file is part of the indismo software. 
#  It is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by 
#  the Free Software Foundation, either version 3 of the License, or any 
#  later version.
#  The software is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#  You should have received a copy of the GNU General Public License,
#  along with the software. If not, see <http://www.gnu.org/licenses/>.
#  see http://www.gnu.org/licenses/.
#
#  Reference: Willem L, Stijven S, Tijskens E, Beutels P, Hens N and 
#  Broeckhove J. (2015) Optimizing agent-based transmission models for  
#  infectious diseases, BMC Bioinformatics.
#
#  Copyright 2015, Willem L, Stijven S & Broeckhove J
#############################################################################
#
# RESULTS AND TIMINGS FROM INDISMO SIMULATIONS
#
#############################################################################

plot_all_results <- function(data_tag){
  
  
  ######################
  ## GET DATA       ##
  ######################
  
  data	   <- read.table(paste(data_tag,'_output.csv',sep=''),header=TRUE,sep=";",stringsAsFactors=F)
  data_log <-read.table(paste(data_tag,'_log.csv',sep=''),header=FALSE,sep=";")
  
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
  
  
  # ######################
  # ## PLOT SUMMARY  	 	##
  # ######################
  # 
  # # input
  # opt_pop_file      <- unique(data$pop_file)
  # opt_seeding_rate  <- sort(unique(data$seeding_rate))
  # opt_R0            <- sort(unique(data$R0))
  # opt_transm_rate   <- sort(unique(data$transm_rate))
  # opt_immunity_rate <- sort(unique(data$immunity_rate))
  # 
  # # stochastic realisations
  # opt_rng_seed      <- unique(data$rng_seed)
  # num_realisations  <- length(opt_rng_seed) 
  # 
  # # for each population
  # # for each seeding rate
  # # for each R0 AND transmission rate
  # for(pop_file in opt_pop_file)
  # {
  #   for(seeding_rate in opt_seeding_rate)
  #   {
  #     for(immunity_rate in opt_immunity_rate)
  #     {
  #       out_ar    <- data.frame(matrix(0,ncol=length(opt_R0),nrow=num_realisations))
  #       out_time  <- out_ar
  #       for(i in 1:length(opt_R0))
  #       {
  #         sel <- data$pop_file == pop_file & data$seeding_rate == seeding_rate & data$R0 == opt_R0[i] & data$immunity_rate == immunity_rate
  #         out_ar[,i] <- data$AR[sel]
  #         out_time[,i] <- data$total_time[sel]
  #       }
  #       names(out_ar)  <- opt_R0
  #       names(out_time) <- opt_R0
  #       boxplot(out_ar,ylab='attack rate',xlab='R0',main=paste(pop_file,'\n','seeding rate:',seeding_rate))
  #       boxplot(out_time,ylab='total time (s)',xlab='R0',main=paste(pop_file,'\n','seeding rate:',seeding_rate)) 
  #     }    
  #   }
  # }
  
  
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
