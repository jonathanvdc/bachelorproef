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
## TMP

cdata <- read.table(paste0('experiments/exp0_contacts.csv'),sep=',',header=T)
pdata <- read.table(paste0('experiments/exp0_participants.csv'),sep=',',header=T)

# names(contacts)
# names(participants)
# 
# names(pdata)
# pdata <- data.frame(local_id=participants$local_id,part_age=participants$participant_age,part_gender=participants$participant_gender)
# 
# names(cdata)
# names(contacts)
# cdata <- contacts[,c('local_id','cnt_age_mean','cnt_home','cnt_work','cnt_school')]
# names(cdata)[2] <- 'cnt_age'
# cdata$cnt_other <- rowSums(contacts[,c('cnt_transport','cnt_leisure','cnt_other')])
# cdata <- merge(cdata,pdata,by='local_id')
# names(cdata)




if(dim(cdata)[1]>0 && dim(pdata)[1]>0)
{
  
  #names(cdata)
  #names(pdata)
  
  #dim(pdata)
  #dim(cdata)
  
  ## EXPLORE
  #par(mfrow=c(1,1))
  #hist(pdata$part_age)
  
  
  ## people without contacts
  dim(pdata)[1] - length(unique(cdata$local_id))
  flag <- pdata$local_id %in% unique(cdata$local_id)
  table(flag)
  pdata[!flag,]
  
  
  ## SETTINGS 
  L <- 80
  
 flag <- cdata$part_age>=0 
 tag <- 'debug'
plot_cnt_matrix <- function(flag,tag)
  {
    # count contacts
    mij_tbl <- table(cdata$part_age[flag],cdata$cnt_age[flag])
    row_ind <- as.numeric(row.names(mij_tbl)) +1 # age 0 == index 1
    col_ind <- as.numeric(colnames(mij_tbl))  +1 # age 0 == index 1
    mij <- matrix(0,max((L+1),row_ind),max((L+1),col_ind))  
    mij[row_ind,col_ind] <- mij_tbl
    
    # count participant per age
    pdata_flag <- pdata$part_age %in% ((min(row_ind,col_ind):max(row_ind,col_ind))-1)
    p_ages_tbl <- table(pdata$part_age[pdata_flag])
    #tbl <- table(cdata$local_id[flag],cdata$part_age[flag])
    #p_ages_tbl <- colSums(tbl>0)
    row_ind <- as.numeric(names(p_ages_tbl)) +1
    p_ages <- matrix(0,max((L+1),row_ind),1)  
    p_ages[row_ind] <- p_ages_tbl
    
    # remove age>L (age_column L+1)  
    mij <- mij[1:(L+1),1:(L+1)]
    p_ages <- p_ages[1:(L+1)]
    
    # make mij reciproce
    for(i in 1:L)
    {
      for(j in i:L)
      {
        mij[i,j] <- (mij[i,j] + mij[j,i])/2
        mij[j,i] <- mij[i,j]
      }
    }
    
    # adjust for number of participants (if age is present)
    ages_present <- p_ages>0
    p_ages[ages_present]
    mij[ages_present,ages_present] <- mij[ages_present,ages_present]/rep(p_ages[ages_present],each=sum(ages_present)) ## CORRECT??!
    
    # plot matrix 
    image(c(0:L),c(0:L),mij,xlab="age of respondent",ylab="age of contact",main=paste(tag))
    
    # plot number of contacts
    num_cnt_age <- colSums(mij)
    plot(num_cnt_age,main=paste(tag),xlab='age',ylab='count')  
    
    return(mij)
  }
  
  pdf('./cnt_results.pdf')
  par(mfrow=c(2,2))
  ## TOTAL
  mij_total <- plot_cnt_matrix(cdata$part_age>=0,'total')
  
  ## HOUSEHOLD
  mij_hh <- plot_cnt_matrix(cdata$cnt_home==1,'household')
  
  ## WORK
  mij_work <- plot_cnt_matrix(cdata$cnt_work==1,'work')
  
  ## SCHOOL
  mij_school <- plot_cnt_matrix(cdata$cnt_school==1,'school')
  
  ## OTHER
  mij_other <- plot_cnt_matrix(cdata$cnt_other==1,'other')
  dim(mij_other)
  
  ## LOAD FLEMISH SURVEY DATA
  survey_mij_total <- read.table(file='../../data/fl2010_regular_weekday_gam_mij_rec.csv',sep=';',dec=',',header=T)
  survey_mij_hh <- read.table(file='../../data/fl2010_regular_weekday_household_gam_mij_rec.csv',sep=';',dec=',',header=T)
  survey_mij_work <- read.table(file='../../data/fl2010_regular_weekday_workplace_gam_mij_rec.csv',sep=';',dec=',',header=T)
  survey_mij_school <- read.table(file='../../data/fl2010_regular_weekday_school_gam_mij_rec.csv',sep=';',dec=',',header=T)
  survey_mij_other <- read.table(file='../../data/fl2010_regular_weekday_community_gam_mij_rec.csv',sep=';',dec=',',header=T)
  
  ## COMPARE
  par(mfrow=c(3,2))
  
  plot(colSums(survey_mij_total),main='total',xlab='age',ylab='contacts')
  lines(colSums(mij_total),col=2)
  
  plot(colSums(survey_mij_hh),main='household',xlab='age',ylab='contacts')
  lines(colSums(mij_hh),col=2)
  
  plot(colSums(survey_mij_work),main='work',xlab='age',ylab='contacts')
  lines(colSums(mij_work),col=2)
  
  plot(colSums(survey_mij_school),main='school',xlab='age',ylab='contacts')
  lines(colSums(mij_school),col=2)
  
  plot(colSums(survey_mij_other),main='other',xlab='age',ylab='contacts')
  lines(colSums(mij_other),col=2)
  
  dev.off()
  
  ## SCORE
  out <- data.frame(rbind(c(sum(abs(colSums(survey_mij_total)-colSums(mij_total))),
                                sum(abs(colSums(survey_mij_hh)-colSums(mij_hh))),
                                sum(abs(colSums(survey_mij_work)-colSums(mij_work))),
                                sum(abs(colSums(survey_mij_school)-colSums(mij_school))),
                                sum(abs(colSums(survey_mij_other)-colSums(mij_other)))),
                          c(sum(colSums(survey_mij_total)/colSums(mij_total)),
                                sum(colSums(survey_mij_hh)/colSums(mij_hh)),
                                sum(colSums(survey_mij_work)/colSums(mij_work)),
                                sum(colSums(survey_mij_school)/colSums(mij_school)),
                                sum(colSums(survey_mij_other)/colSums(mij_other)))))
  
  out <- round(out)
  names(out) <- paste('score',c('total','household','work','school','other'),sep='_')
  
  write.table(out,file('./cnt_score.csv'),sep=';',dec=',',row.names=F)
  print('Contact processing complete')
 
# end if-clause 
} else {
  print('Contact data incomplete')
}




