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
# SYNTHETIC POPULATIONS
#
# Extract person data from the RTI database and create input files 
# for the indismo project.
# 
#########################
rm(list=ls(all=TRUE))
gc()

#setwd('/home/chermid1/Documents/Lander/synth_pop_US')
#setwd("/Users/lwillem/Documents/Doctoraat/Indismo/US_synt_pop_v2/")


######################
## GET DATA 	 	##
######################

getPDF = TRUE;

# BROOKLYN, NY
data_tag <- "2010_ver1_36047"
geo_tag <- "brooklyn"

# # NASSAU, NY
# data_tag<-"2010_ver1_36059"
# geo_tag <- "Nassau" 

# # NY
# data_tag<-"2010_ver1_36"
# geo_tag <- "NY" 



# GET DATA
data<-read.table(paste(data_tag,"/",data_tag,"_synth_people.txt",sep=""),header=TRUE,sep=",",stringsAsFactors=FALSE)
 
names(data)
data <- data[,c(1,2,5,10,11)]

dataOrig <- data
data <-dataOrig

dim(data)
popSize <- dim(data)[1]
names(data)

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
	pdf(file=paste(geo_tag,"_synt_pop_plot.pdf",sep=""),width=9,height=9)
#	par(mfrow=c(1,4))
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
tmp <- table(table(data$sp_school_id[!is.na(data$sp_school_id)]))
names(tmp)[c(1,length(tmp))]
plot(tmp,main=paste(geo_tag,': school size',sep=""),xlab='size',ylab='freq')

# workplace size distribution
tmp <- table(table(data$sp_work_id[!is.na(data$sp_work_id)]))
#tmp <- table(table(data$hub_id[!is.na(data$hub_id)]))
names(tmp)[c(1,length(tmp))]
plot(tmp,main=paste(geo_tag,': workplace size',sep=""),xlab='size',ylab='freq')



######################
## MERGE DAY HUBS 	##
######################

unq_school_id <- unique(data$sp_school_id[!is.na(data$sp_school_id)])
unq_workplace_id <- unique(data$sp_work_id[!is.na(data$sp_work_id)])
length(unq_school_id) + length(unq_workplace_id)
length(unique(c(unq_school_id, unq_workplace_id)))

# merge if school and workplace id's are different
data$hub_id <- data$sp_school_id
data$hub_id[!is.na(data$sp_work_id)] <- data$sp_work_id[!is.na(data$sp_work_id)]

length(unique(data$hub_id[!is.na(data$hub_id)]))

# day hub size distribution
tmp <- table(table(data$hub_id[!is.na(data$hub_id)]))
names(tmp)[c(1,length(tmp))]
#plot(tmp,main=paste(geo_tag,': day hub size'),xlab='size',ylab='freq')

hub_stat <- matrix(NA,ncol=2,nrow=6)
hub_stat_legend <- matrix(NA,ncol=1,nrow=dim(hub_stat)[1])
hub_stat[,1] <- c(1,2,6,21,101,1001)

hub_stat[1,2] <- sum(tmp[as.numeric(names(tmp))<hub_stat[2,1]])
hub_stat_legend[1] <- hub_stat[1,1]

for(i in 2:(dim(hub_stat)[1]-1)){
	hub_stat[i,2] <- sum(tmp[as.numeric(names(tmp))>=hub_stat[i,1] & as.numeric(names(tmp))<hub_stat[i+1,1]])
	hub_stat_legend[i] <- paste(hub_stat[i,1],hub_stat[i+1]-1,sep="-")
}
hub_stat[6,2] <- sum(tmp[as.numeric(names(tmp))>hub_stat[6,1]])
hub_stat_legend[6] <- paste('+',hub_stat[6,1],sep="")

row.names(hub_stat) <- hub_stat_legend
barplot(hub_stat[,2], main=paste('day hub size',sep=""),xlab='size',ylab='freq')
legend(x=4,y=max(hub_stat),paste('max size:', row.names(tmp)[length(tmp)]))


##########################
## REFORMAT ID's 		##
##########################

## HELPER FUNCTION
#f_original_id <- data$hub_id
getSortedId <- function(f_original_id){
	
	# set smallest id to "1"
	new_id <- f_original_id - min(f_original_id,na.rm=T) +1
	
	# make id from 1 to nb_unique ids
	# get sequence of id's
	order_id <- order(new_id)
	# give incremental id to the sorted id's
	order_new_id <- cumsum(c(1,diff(new_id[order_id])>0))
	# copy new id's back to the original vector
	new_id[order_id] <- order_new_id
	
	return(new_id)
}

# id ranges
c(min(data$sp_id),max(data$sp_id))
c(min(data$sp_hh_id),max(data$sp_hh_id))
c(min(data$hub_id,na.rm=T),max(data$hub_id,na.rm=T))

data$sp_id <- getSortedId(data$sp_id)
data$sp_hh_id <- getSortedId(data$sp_hh_id)
data$hub_id <- getSortedId(data$hub_id)

# id ranges
c(min(data$sp_id),max(data$sp_id))
c(min(data$sp_hh_id),max(data$sp_hh_id))
c(min(data$hub_id,na.rm=T),max(data$hub_id,na.rm=T))


# tmp <- table(data$sp_hh_id)
# tmp <- table(data$hub_id)
# tmp <- table(data$home_nbh)
# tmp <- table(data$day_nbh)

# c(min(tmp),max(tmp),mean(tmp),median(tmp))
# dim(data)

##########################
## CREATE NEIGHBORHOODS ##
##########################

f_cluster_id <- data$sp_hh_id
f_popSize <- dim(data)[1]
names(data)

# FUNCTION: get neighborhood based on given cluster_id's. Start from id 1 and loop untill max id and start new neighborhood if maximum number of members is reached.
getNbh <- function(f_cluster_id,f_popSize){

nbhSize = 2000;
maxNbDistricts = round(f_popSize / nbhSize) +1


# 1. GET FIRST AND LAST cluster_id FOR EACH NEIGHBORHOOD
nbh_start = rep(0,maxNbDistricts)
nbh_end = rep(0,maxNbDistricts) 

sizes <- table(f_cluster_id[f_cluster_id>0])
counter <- 0
nbhId <- 1;
nbh_start[1] <- 1    #start with first cluster_id
i<-2
for(i in 1:length(sizes)){

 	counter = counter + sizes[i]

if(counter > nbhSize){
	counter <- 0;
	
	nbh_end[nbhId] = i+1;
	nbh_start[nbhId+1] = i+1;
	
	nbhId = nbhId + 1;	
}	
}
nbh_end[nbhId] <- length(sizes)+1 # end with last cluster_id

nbh_start[1:10]
nbh_end[1:10]
nbh_start==nbh_end
nbh_end - nbh_start
						
					 
# ASSIGN THE cluster_id's TO THEIR NBH
nbh_id = rep(NA,popSize)
for(i in 1:maxNbDistricts){
	flag <- f_cluster_id >= nbh_start[i] & f_cluster_id < nbh_end[i]
	sum(flag)
	nbh_id[flag] = i
	
}
sum(!is.na(nbh_id))

nbh_id[20:30]
length(nbh_id)
f_cluster_id[20:30]


# RETURN
return(nbh_id)
}

# GET HOME NBH
data$home_nbh <- getNbh(data$sp_hh_id,popSize)
tmp <- table(table(data$home_nbh))
#plot(tmp,main=paste('geo_tag,': 'home neighborhood size',sep=""),xlab='size',ylab='freq')
barplot(tmp,main=paste('home neighborhood size',sep=""),xlab='size',ylab='freq',cex.names=0.8)


# GET DAY NBH
data$day_nbh <- getNbh(data$hub_id,popSize)
tmp <- table(table(data$day_nbh[data$day_nbh>0]))
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
barplot(hub_stat[,2], main=paste('day neigborhood size',sep=""),xlab='size',ylab='freq')
legend(x=4,y=max(hub_stat[,2]),paste('max size:', row.names(tmp)[length(tmp)]))


# ######################
# ## DOUBLE DATASET	##
# ######################

# data2 <- data
# names(data2)
# data2$p_id <- data2$p_id + max(data$sp_id,na.rm=T)
# data2$hh_id <- data2$hh_id + max(data$sp_hh_id,na.rm=T)
# data2$home_nbh <- data2$home_nbh + max(data$home_nbh,na.rm=T)
# data2$hub_id <- data2$hub_id + max(data$hub_id,na.rm=T)
# data2$day_nbh <- data2$day_nbh + max(data$day_nbh,na.rm=T)


# data3 <- rbind(data,data2)
# dim(data)
# dim(data2)
# dim(data3)

# data <- data3
# geo_tag <- "allegheny2" 

# # id ranges
# c(min(data$sp_id),max(data$sp_id))
# c(min(data$sp_hh_id),max(data$sp_hh_id))
# c(min(data$hub_id,na.rm=T),max(data$hub_id,na.rm=T))



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
par(mfrow=c(4,1))
#plot(f_out$p_id,main='person id'); abline(0,0,0,c(which(steps==1)),lty=3)
plot(f_out$hh_id,main='hh id'); abline(0,0,0,c(which(steps==1)),lty=3)
plot(f_out$hub_id,main='hub id') ;abline(0,0,0,c(which(steps==1)),lty=3)
plot(f_out$home_nbh,main='home nbh') ;abline(0,0,0,c(which(steps==1)),lty=3)
plot(f_out$day_nbh,main='day nbh') ;abline(0,0,0,c(which(steps==1)),lty=3)

par(mfrow=c(1,1))
}

#-------------------------------------------------------------

# out <- data.frame(cbind(data$age,data$sp_hh_id,data$hub_id))
# names(out) <- c("age","hh_id","hub_id")
out <- data.frame(cbind(data$age,data$sp_hh_id,data$hub_id,data$home_nbh,data$day_nbh))
names(out) <- c("age","hh_id","hub_id","home_nbh","day_nbh")

# set NA id's to "0"
out[is.na(out)] <- 0


dim(out)
out[1:10,]
plotIndices(out, out$p_id,40)


# ORIGINAL ORDER
out_new <- out
plotIndices(out_new, out_new$hh_id,-3000)
write.table(out_new,file=paste(geo_tag,"_synt_pop_orig.csv",sep=""),sep=";",row.names=FALSE)

# SORT: hub, household
index <- with(out, order(hub_id, hh_id))
out_new <- out[index,]
plotIndices(out_new, out_new$hub_id,-4000)
write.table(out_new,file=paste(geo_tag,"_synt_pop_hub_hh.csv",sep=""),sep=";",row.names=FALSE)

# SORT: household, hub
index <- with(out, order(hh_id, hub_id))
out_new <- out[index,]
plotIndices(out_new, out_new$hh_id,-4000)
write.table(out_new,file=paste(geo_tag,"_synt_pop_hh_hub.csv",sep=""),sep=";",row.names=FALSE)

# RANDOM
set.seed(4)
index <- sample(1:dim(out)[1])
out_new <- out[index,]
plotIndices(out_new, out_new$hh_id,-3000)
write.table(out_new,file=paste(geo_tag,"_synt_pop_rand.csv",sep=""),sep=";",row.names=FALSE)

# SORT hh, RANDOM hub
set.seed(4)
index <- sample(1:dim(out)[1])
out_new <- out[index,]
index <- with(out, order(hh_id))
out_new <- out[index,]
plotIndices(out_new, out_new$hh_id,-3000)
write.table(out_new,file=paste(geo_tag,"_synt_pop_hh.csv",sep=""),sep=";",row.names=FALSE)

# SORT hub, RANDOM hh
set.seed(4)
index <- sample(1:dim(out)[1])
out_new <- out[index,]
index <- with(out, order(hub_id))
out_new <- out[index,]
plotIndices(out_new, out_new$hh_id,-3000)
write.table(out_new,file=paste(geo_tag,"_synt_pop_hub.csv",sep=""),sep=";",row.names=FALSE)




##########################
## NEIGHBORHOODS (GEO)		##
##########################
# function from FRED
get_x <- function(f_longitude){
	km_per_deg_longitude  = 87.832; # for the USA at 38 deg N
	
	return(round((f_longitude + 180) * km_per_deg_longitude))
}

# function from FRED
get_y <- function(f_latitude){
	km_per_deg_latitude = 110.996 # for the USA at 38 deg N
	return(round((f_latitude + 90.0) * km_per_deg_latitude))
}


# new function: get neighborhood id
get_nbh <- function(f_longitude,f_latitude,f_grid_interval,f_x_max){
	
	x_coord <-get_x(f_longitude)
	y_coord <-get_x(f_latitude)
	#plot(hh_data$x_coord,hh_data$y_coord)

	# assign coordinates on grid
	x_start <- (x_coord - x_coord%%f_grid_interval)
	y_start <- (y_coord - y_coord%%f_grid_interval)

	# get nhb_id
	nbh_id <- x_start + (f_x_max*(y_start-1))
	
	# return
	return(nbh_id)
	
}

x_max <- 5000
grid_interval <- 5

# Households
hh_data<-read.table(paste(data_tag,"/",data_tag,"_synth_households.txt",sep=""),header=TRUE,sep=",",stringsAsFactors=FALSE)

dim(hh_data)
plot(hh_data$longitude,hh_data$latitude,xlab='longitude',ylab='latitude')

max(get_x(hh_data$longitude))
hh_data$hh_nbh <- get_nbh(hh_data$longitude,hh_data$latitude,10,4930)

table(table(hh_data$hh_nbh,useNA="ifany"),useNA="ifany")
max(hh_data$hh_nbh)
dim(hh_data)



#_________________

if(getPDF){dev.off()} #close pdf stream

