# Calculate the coordinates of a circular path at time t with speed s [0, 1].
lambda r, s, t: [900 + r*80*cos(t*0.04*s), -60 + r*80*sin(t*0.04*s), 2]

