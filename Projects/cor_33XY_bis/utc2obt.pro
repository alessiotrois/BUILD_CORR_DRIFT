function mjdtime,time

result={mjd:0l,time:0l}

secs=long(time(0))*long(3600)+long(time(1)*60)+long(time(2))
msod=secs*long(1000)+long(time(3))
y=long(time(6))
m=long(time(5))
d=long(time(4))
l=(m-14)/12
result.mjd=d-2432076L+1461*(y+4800+l)/4+367*(m-2-l*12)/12-3*((y+4900+l)/100)/4
result.time=msod

return,result
end

pro utc2obt

start_date0=''
print,''
read,start_date0,prompt='UTC CALENDAR TIME (DD/MM/YYYY hh:mm:ss.sss)= '
print,''
sdate=fix(strsplit(start_date0,' ./:',/extract))
sdate_arr=[sdate[3],sdate[4],sdate[5],sdate[6],sdate[0],sdate[1],sdate[2]]
smjd0=mjdtime(sdate_arr)
start_mjd=double(smjd0.mjd)+double(smjd0.time)/86400000.d
start_time=(start_mjd-53005.)*86400.d
print,''
print,'AGILE OBT TIME: '+strtrim(string(start_time,format='(d16.5)'),2)
print,''
end
