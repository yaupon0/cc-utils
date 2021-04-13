--[[
    Simulation of Paxos multi-proposer to show livelock

    Fixed number (T.proposers) of proposers. 
    Starting with T.acceptor acceptors, and increments by 2 each time until it reaches max 
    Do{
    Run T.rounds of the simulation
    Each round has T.timeperround moves (one randomly chosen proposer attempting to advance)
    There is a T.dropprob probability that the selected proposer can't communicate this move

    If the proposer can move it contacts one previously not contacted
    acceptor (unless it has struck out with all of them). If it is in phase 1 it looks for an acceptor to accept its
    current id if it is in phase 2 it looks for an acceptor to approve its proposal
    contacting one acceptor per move
    }

    Probability of livelock increases with T.dropprob (steeply) and with number of acceptors less steeply





    (c) Victor Yodaiken 2016-2021.  Permission is hereby granted,
    free of charge,
    to any person obtaining a copy of this software and associated
    documentation files (the "Software"),
    to deal in the Software without restriction,
    including without limitation the rights to use, copy, modify, merge,
    publish, distribute, sublicense, and / or sell copies of the Software,
    and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.THE SOFTWARE IS
    PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
    INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
    IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
    CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
    TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE. ]]


-- local _ENV = require 'std.strict' (_G)
  -- - globals for the game
T= {
rounds = 50000,
proposers = 5,
acceptors = 7, --- initial number of acceptors
timeperround = 1000,  --- one move per time unit
maxacceptors=35,
dropprob = 0.001 --- probability message gets dropped
}

local p = { }
local a = { } 
local stat = {}
function count (x, n)-- - how many non - nil entries up to n are in array
	local c = 0
	for i = 1, n do
		if x [i] ~= nil then c = c + 1 end
	end
	return c;
end

function displayproposers (m, axes)-- print out proposer key values
    local v = nil print (m, "-------------------")
    for i = 1, T.proposers do
	print (i, "id=", p[i].id, "phase=",
	       p[i].phase, "v=", p[i].value, count (p[i].approved, axes),
	       count (p[i].accepted, axes))
	       if p[i].phase == 3 then
		       if v == nil then v = p[i].value
		       elseif v ~= p[i].value then print ("failed")
		       end
	       end
       end
end

function statinit (numberofacceptors)
	stat = {
		runs = 0,
		failures = 0,
		livelocks = 0,
		livelockid = 0,
		winners = 0,
		winning = 0,
		acceptors = numberofacceptors,
		sumofvalues = 0,
		timedoutphase1 = 0,
		timedoutphase2 = 0
	}
end

function highestid ()
	local bestid = 0
	for i = 1, T.proposers do
		if p[i].id > bestid then bestid = p[i].id end
	end
	return bestid
end

function stats (axes)
	local winners = 0
	local value = nil
	local fails = 0

	for i = 1, T.proposers do
		if p[i].phase == 3 then
			winners = winners + 1
			if value == nil then value = p[i].value
			elseif value ~= p[i].value then fails = 1
			end
		end
	end
	stat.runs = stat.runs + 1
	if winners == 0 then
		stat.livelocks = stat.livelocks + 1
		
		if (stat.livelockid < highestid ()) then stat.livelockid = highestid ()end
		else
		stat.winners = stat.winners + winners
		stat.winning = stat.winning + 1
		stat.sumofvalues = stat.sumofvalues + value
	end
	if fails == 1 then
		    stat.failures = stat.failures + 1
		    displayproposers ("consensus failure", axes)
		    end
end

function displaystats ()
	print ("This test ----------------------\n")
	print ("Acceptors=", stat.acceptors, "Average winning id=", string.format ("%.2f", stat.  sumofvalues/stat.winning),
			   "\nChance of livelock=", stat.livelocks / stat.runs, "Rounds without consensus=",
			   T.rounds - stat.winning, "Highest id on livelock ", stat.livelockid)
			   print("Time outs in phase 1",stat.timedoutphase1, "phase 2", stat.timedoutphase2)
end


function pickvalue (n)-- - values are the ids of the proposer
	local v
	if p[n].bestv ~= nil then v = p[n].bestv
	else v = p[n].id end
	return v
end

function newapproval (px, numa)
	local ax;
	-- try to find an acceptor  that will talk
	for ax=1,numa do
		if p[px].contacted[ax] ~= true then 
			p[px].contacted[ax] = true
			if (p[px].approved[ax] ~= true and a[ax].highestq <= p[px].id ) then -- ifq
		--		print(px, p[px].id, "is approved by",ax)
				p[px].approved[ax] = true;
				a[ax].highestq = p[px].id
				if p [px].bestq < a[ax].bestq then
					p[px].bestq = a[ax].bestq
					p[px].bestv = a[ax].bestv
				end
				if count(p[px].approved, numa) > numa / 2 then -- ifcount
					p[px].phase = 2
					p[px].value = pickvalue (px)
					p[px].contacted = {}
				end -- ifcount
				return true
			end-- ifq
		end
	end
	-- at this point we've contacted everyone and can't win, so reset with higher id
		initonep (px)
		p[px].id = p[px].id + T.proposers
		stat.timedoutphase1 = stat.timedoutphase1 +1
		return
end


function newaccept (px, numa)
	-- try to find an acceptor  that likes us 
	for ax=1,numa do
		if p[px].contacted[ax] ~= true then 
			p[px].contacted[ax]= true
			if (p[px].accepted[ax] ~= true and a[ax].highestq <= p[px].id) then-- ifq
				p[px].accepted[ax] = true ;
				if a[ax].bestq < p[px].id then
					a[ax].bestq = p[px].id
					a[ax].bestv = p[px].value
				end
				if count(p[px].accepted, numa) > (numa / 2) then
					p[px].phase = 3
					p[px].contacted = {}
					-- print(px,p[px].id,  " wins");
				end -- count
				return
			end-- ifq
		end -- end of for loop.
	end
	-- at this point we've contacted everyone and can't win, so reset with higher id
--	print(px,p[px].id,"failed and increment id")
		stat.timedoutphase2 = stat.timedoutphase2 +1
		initonep (px)
		p[px].id = p[px].id + T.proposers
		return
	end

function initp ()
	for i = 1, T.proposers do initonep (i) end
end

function initonep (i)
	p[i] = { id = i,  phase = 1, contacted ={},
	approved = {}, accepted = {}, value = 0, bestv = nil, bestq = 0 }
end

function inita ()
	for i = 1, T.maxacceptors do
		a[i] = { id = i, highestq = 0, bestv = nil, bestq = 0 }
	end
end

--			main
--
	math.randomseed (os.time ())
	print ("Simulate Paxos Consensus with increasing numbers of acceptors\n");
	print ("Proposers=", T.proposers, "\nTime per round =", T.timeperround, " Rounds=", T.rounds )
	for n= T.acceptors, T.maxacceptors, 2 do
			  --foracceptors
			  statinit(n)
			  for round = 1, T.rounds do --forround
				  initp ();
				  inita ();
				  for t = 1, T.timeperround do --fortime
					  local px = math.random (T.proposers)
					  if p [px].phase == 1 and math.random()> T.dropprob then newapproval (px, n)
					  elseif p[px].phase == 2 and math.random()> T.dropprob  then newaccept(px, n)
					  else break; -- someone won
					  end
				  end-- fortime
				  stats (n)
			  end
			  displaystats()
		  end
		  print("Ax\tLlock%\tWinning\tAvg value")
