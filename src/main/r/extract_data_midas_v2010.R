#############################################################################
#  This file is part of the Stride software. 
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
#  Copyright 2016, Willem L, Kuylen E & Broeckhove J
#############################################################################
# SYNTHETIC POPULATIONS
#
# Extract person data from the RTI database and create input files 
# for the stride project.
# 
# Version 3
#
# Author: Lander Willem
# Last update: 23/01/2017  
#########################
rm(list=ls(all=TRUE))
gc()

#setwd('./synth_pop_US')


######################
## GET DATA 	 	##
######################

getPDF = TRUE;

# BROOKLYN, NY
#data_tag <- "2010_ver1_36047"
#geo_tag <- "brooklyn"

# NASSAU, NY
#   data_tag<-"2010_ver1_36059"
#   geo_tag <- "nassau_v3" 

# # NY
# data_tag<-"2010_ver1_36"
# geo_tag <- "NY"

# UTAH, TEXAS
#data_tag <- "2010_ver1_49"
#geo_tag <- "utah"

# # MIAMI, FLORIDA
data_tag <- "2010_ver1_12086"
geo_tag <- "miami_stride"

# # FLORIDA
# data_tag <- "2010_ver1_12"
# geo_tag <- "florida" 


# GET DATA
data<-read.table(paste(data_tag,"/",data_tag,"_synth_people.txt",sep=""),header=TRUE,sep=",",stringsAsFactors=FALSE)

names(data)
data <- data[,c(1,2,5,10,11)]

dim(data)
popSize <- dim(data)[1]
names(data)

##############################################
## OVERLAPPING SCHOOL AND WORK PRESENCE  	 	##
##############################################

# Some people (#8617) from age 16y-18y have school and workplace id
# FIX: assign them to the workplace
sum(!is.na(data$sp_school_id[!is.na(data$sp_work_id)]))
table(data$age[!is.na(data$sp_school_id) & !is.na(data$sp_work_id)])
data$sp_school_id[!is.na(data$sp_work_id)] <- NA


######################
## EXPLORE		 	##
######################

# open pdf stream
if(getPDF){
  pdf(file=paste("pop_",geo_tag,"_plot.pdf",sep=""),width=9,height=9)
}

#pop size (metadata alaska: 620490)
dim(data)[1]
# nb households (metadata alaska: 234779)
length(unique(data$sp_hh_id[!is.na(data$sp_hh_id)])) 
# nb schools (metadata alaska: 560)
length(unique(data$sp_school_id[!is.na(data$sp_school_id)]))
# nb workplaces (metadata alaska: 32609)
length(unique(data$sp_work_id[!is.na(data$sp_work_id)]))

# age distribution
tmp <- table(data$age)
names(tmp)[c(1,length(tmp))]
barplot(tmp,main=paste('ages',sep=""),xlab='age',ylab='freq')

# hh size distribution
tmp <- table(table(data$sp_hh_id[!is.na(data$sp_hh_id)]))
names(tmp)[c(1,length(tmp))]
barplot(tmp,main=paste('household size',sep=""),xlab='size',ylab='freq')

# school size distribution
tmp_school <- table(table(data$sp_school_id[!is.na(data$sp_school_id)]))
names(tmp_school)[c(1,length(tmp_school))]
plot(tmp_school,main=paste(geo_tag,': school size',sep=""),xlab='size',ylab='freq')

# workplace size distribution
tmp_work <- table(table(data$sp_work_id[!is.na(data$sp_work_id)]))
#tmp <- table(table(data$hub_id[!is.na(data$hub_id)]))
names(tmp_work)[c(1,length(tmp_work))]
plot(tmp_work,main=paste(geo_tag,': workplace size',sep=""),xlab='size',ylab='freq')

##########################
## REFORMAT ID's     ##
##########################

## HELPER FUNCTION
f_original_id <- data$home_district
getSortedId <- function(f_original_id){
  
  # set smallest id to "1"
  new_id <- f_original_id - min(f_original_id,na.rm=T) 
  
  # make id from 0 to nb_unique_ids-1
  # get sequence of id's
  order_id <- order(new_id)
  # give incremental id to the sorted id's
  order_new_id <- cumsum(c(1,diff(new_id[order_id])>0))
  # copy new id's back to the original vector
  new_id[order_id] <- order_new_id
  
  return(new_id)
}

# id ranges, initial
c(min(data$sp_id),max(data$sp_id))
c(min(data$sp_hh_id),max(data$sp_hh_id))
c(min(data$sp_school_id,na.rm=T),max(data$sp_school_id,na.rm=T))
c(min(data$sp_work_id,na.rm=T),max(data$sp_work_id,na.rm=T))

# adjust: start with id 0
data$sp_id <- getSortedId(data$sp_id)
data$sp_hh_id <- getSortedId(data$sp_hh_id)
data$sp_school_id <- getSortedId(data$sp_school_id)
data$sp_work_id <- getSortedId(data$sp_work_id)

# id ranges, final
c(min(data$sp_id),max(data$sp_id))
c(min(data$sp_hh_id),max(data$sp_hh_id))
c(min(data$sp_school_id,na.rm=T),max(data$sp_school_id,na.rm=T))
c(min(data$sp_work_id,na.rm=T),max(data$sp_work_id,na.rm=T))


##########################
## CREATE DISTRICTS     ##
##########################

f_cluster_id <- data$sp_hh_id
f_cluster_id <- data$sp_work_id

f_popSize <- dim(data)[1]
names(data)

# FUNCTION: get neighborhood based on given cluster_id's. Start from id 1 and loop untill max id and start new neighborhood if maximum number of members is reached.
getDistrict <- function(f_cluster_id,f_popSize){
  
  districtSize = 2000;
  maxNbDistricts = round(f_popSize / districtSize)
  
  
  # 1. GET FIRST AND LAST cluster_id FOR EACH NEIGHBORHOOD
  district_start = rep(0,maxNbDistricts)
  district_end = rep(0,maxNbDistricts) 
  
  sizes <- table(f_cluster_id)
  counter <- 0
  districtId <- 1;
  district_start[1] <- 0    #start with first cluster
  i<-597
  for(i in 1:length(sizes)){
    
    counter = counter + sizes[i]
    
    if(counter > districtSize){
      counter <- 0;
      district_end[districtId] = i;
      
      # next neighborhood
      districtId = districtId + 1;
      district_start[districtId] = i+1;
      
    }	
  }
  district_end[districtId] <- length(sizes) # end with last cluster_id
  
  district_start[1:10]
  district_end[1:10]
  district_start==district_end
  district_end - district_start
  
  
  # ASSIGN THE cluster_id's TO THEIR district
  district_id = rep(NA,popSize)
  i<-1
  for(i in 1:maxNbDistricts){
    flag <- f_cluster_id >= district_start[i] & f_cluster_id <= district_end[i]
    sum(flag)
    district_id[flag] = i
    
  }
  sum(!is.na(district_id))
  sum(is.na(district_id))
  
  # RETURN
  return(district_id)
}

# GET HOME district
data$home_district <- getDistrict(data$sp_hh_id,popSize)
tmp <- table(table(data$home_district))
#plot(tmp,main=paste('geo_tag,': 'home neighborhood size',sep=""),xlab='size',ylab='freq')
barplot(tmp,main=paste('home neighborhood size',sep=""),xlab='size',ylab='freq',cex.names=0.8)


# GET DAY district = HOME district
data$day_district <- data$home_district                                                           # NEW !!!
tmp <- table(table(data$day_district[data$day_district>0]))
#plot(tmp,main=paste(geo_tag,': day neighborhood size',sep=""),xlab='size',ylab='freq')
#barplot(tmp,main=paste('home neighborhood size',sep=""),xlab='size',ylab='freq')


hub_stat <- matrix(NA,ncol=2,nrow=5)
hub_stat_legend <- matrix(NA,ncol=1,nrow=dim(hub_stat)[1])
hub_stat[,1] <- c(2000,2000,2101,3001,4001)

hub_stat[1,2] <- sum(tmp[as.numeric(names(tmp))<hub_stat[2,1]])
hub_stat_legend[1] <- paste('<',hub_stat[1,1],sep="")

for(i in 2:(dim(hub_stat)[1]-1)){
  hub_stat[i,2] <- sum(tmp[as.numeric(names(tmp))>=hub_stat[i,1] & as.numeric(names(tmp))<hub_stat[i+1,1]])
  hub_stat_legend[i] <- paste(hub_stat[i,1],hub_stat[i+1]-1,sep="-")
}
hub_stat[5,2] <- sum(tmp[as.numeric(names(tmp))>hub_stat[5,1]])
hub_stat_legend[5] <- paste('>',hub_stat[4,1],sep="")

row.names(hub_stat) <- hub_stat_legend
barplot(hub_stat[,2], main=paste('day district size',sep=""),xlab='size',ylab='freq')
legend(x=4,y=max(hub_stat[,2]),paste('max size:', row.names(tmp)[length(tmp)]))


############################
## REFORMAT DISTRICT ID's	##
############################

# id ranges, initial
c(min(data$home_district),max(data$home_district))
c(min(data$day_district),max(data$day_district))

# adjust: start with id 0
data$home_district <- getSortedId(data$home_district)
data$day_district <- getSortedId(data$day_district)


# id ranges, final
c(min(data$home_district),max(data$home_district))
c(min(data$day_district),max(data$day_district))


##########################
## STORE DATASET 		##
##########################
#-------------------------------------------------------------
# f_out <- out_new
# f_basis <- out_new$hh_id
# f_nb <- -20
plotIndices <- function(f_out, f_basis, f_nb){
  
  f_size <- dim(f_out)[1]
  if(f_nb>0){
    f_out <- f_out[1:f_nb,]
  } else {
    f_out <- f_out[(f_size+f_nb):f_size,]
  }
  steps <- c(1,diff(f_basis))
  if(abs(f_nb)>100){
    steps <- 0
  }
  par(mfrow=c(2,3))
  plot(f_out$hh_id,main='hh id'); abline(0,0,0,c(which(steps==1)),lty=3)
  plot(f_out$school_id,main='school id') ;abline(0,0,0,c(which(steps==1)),lty=3)
  plot(f_out$work_id,main='work id') ;abline(0,0,0,c(which(steps==1)),lty=3)
  plot(f_out$home_district,main='home district') ;abline(0,0,0,c(which(steps==1)),lty=3)
  plot(f_out$day_district,main='day district') ;abline(0,0,0,c(which(steps==1)),lty=3)
  
  par(mfrow=c(1,1))
}

#-------------------------------------------------------------

out <- data.frame(cbind(data$age,data$sp_hh_id,data$sp_school_id,data$sp_work_id,data$home_district,data$day_district))
dim(out)
names(out) <- c("age","hh_id","school_id","work_id","home_district","day_district")

# set NA id's to ""
out[is.na(out)] <- 0

# ORIGINAL ORDER
out_new <- out
plotIndices(out_new, out_new$hh_id,-3000)
write.table(out_new,file=paste("pop_",geo_tag,"_orig.csv",sep=""),sep=",",row.names=FALSE)

# SORT: home_district, household
index <- with(out, order(home_district, hh_id))
out_new <- out[index,]
plotIndices(out_new, out_new$home_district,-4000)
write.table(out_new,file=paste("pop_",geo_tag,"_home_hh.csv",sep=""),sep=",",row.names=FALSE)

# # RANDOM
# set.seed(4)
# index <- sample(1:dim(out)[1])
# out_new <- out[index,]
# plotIndices(out_new, out_new$hh_id,-3000)
# write.table(out_new,file=paste("pop_",geo_tag,"_rand.csv",sep=""),sep=",",row.names=FALSE)

dev.off()

