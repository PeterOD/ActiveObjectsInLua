--[[
format of message queue:

queue = { id = " foo", msgs = {m1 = " Bar",m2 = "content"...mn = "content"}}

]]

 active_object = require "loop.simple"	--remove "local"

 AO = active_object.class{

 }


 function AO:Construct(name)
	self.name = name
end

function AO:SetName(v)

	if type(v) == "string" then
	self.name = v
	else
		error("Cannot assign name")
	end
end


function AO:Getname()

	return self.name
end

function AO:CreateMSG(objID,msg)
	m = newElem(m)
	m.id = objID
	if type(msg) =="table" then
		m.msgs =msg
	end
	return m

end



function AO:SendMSG(val)
if(v == nil) then return end
if type(val) ~= "table" then
	error("Doomed") end
tbl:Add(val)
end

--table to hold 'linked list'
local tbl = {}

--table that will hold objects in use
 in_use = {}

function in_use:CheckID(val)
	for _,var in ipairs(in_use) do
		if var == val then
			in_table = true
		else
			in_table = false
		end
	end
	return in_table
end

function in_use:Add(v)

	table.insert(in_use,v)

end

-- This function will create a new element -- a table
 function newElem(v)
        local link = {}

        link.__o = v
--		print(type(link.__o).." --> "..type(link))

        return link
end
--Initialise the table

function tbl:Init()
        if(self.inited ~= true) then
                self.inited = true
        else
                return end
        self.count = 0
        self.start = nil
        self.current = nil
end

---- GETTER functions --------------
local function GetLinkAtIndex(self,index)
        if(self.start == nil) then return end
        if(index == nil) then return end
        if(index <= 0) then error("index Out Of Bounds " .. index .. " <= 0") return end
        local l = self.start
        local i = 1
        while(l.next ~= nil and i < index) do
                l = l.next
                i = i + 1
        end
        if(i < index) then error("index Out Of Bounds " .. index .. " > " .. i) return end
        return l
end

function tbl:GetLink(index)
        return GetLinkAtIndex(self,index)
end

function tbl:Get(index)
        local l = GetLinkAtIndex(self,index)
        if(l == nil) then return end
        return l.__o
end
------------------------------------

function tbl:Add(v)
        if(v == nil) then return end
        local link = newElem(v)
        if(self.start == nil) then
                self.start = link
                self.last = link
                self.count = 1
        else
                if(self.last == nil) then return end
                self.last.next = link
                link.prev = self.last
                self.last = link
                self.count = self.count + 1
        end
end

--- //Sort the table

function tbl:Sort(compare)
        if(self.start == nil) then return end
        local temp, temp1, temp2
        local d
        local head = self.start

        temp=head
        temp1=head

        while(temp.next ~= nil) do
                temp=temp.next
                while(temp1.next ~= nil) do
                        temp2=temp1
                        temp1=temp1.next
                        local b,e = pcall(comparator,temp2.__o,temp1.__o)
                        if(b == nil) then error("problem with arg to function tbl:Sort(): " .. e .. "\n") end

                        if(e) then
                                d = temp2.__o
                                temp2.__o = temp1.__o
                                temp1.__o = d
                        end
                end

                temp2=head
                temp1=head
        end
end



function tbl:Len()
        return self.count
end

function tbl:IsEmpty()
	if self.count == 0 then
	return true
	else
	return false
	end
end

function tbl:Iter(func)
        if(type(func) ~= "function") then return end
        if(self.start == nil) then return end
        local i = 1
        local l = self.start
        while(l.next ~= nil) do
                if(self.removeCall == true) then
                        i = i - 1
                        self.removeCall = false
                end

                self.current = l
                if(i > self:Len() or i <= 0) then return end
                local b,e = pcall(func,l.__o,i)
                self.current = nil
                if(b ~= true) then error(e) end
                l = l.next
                i = i + 1
        end

        self.current = l
        if(i > self:Len() or i <= 0) then return end
        local b,e = pcall(func,l.__o,i)
        self.current = nil
        if(b ~= true) then error(e) end
end

--- Constructor for queue ------
function TBL()
        local o = {}

        setmetatable(o,tbl)
        tbl.__index = tbl

        o:Init()

        return o;
end








--[[
--------------------------------------------------------------------------------
-- "IMPLEMENTATION"
--
-------------------------------------------------------------------------------
obj = active_object.class({},AO)
obj:Construct()
obj:SetName("JayBo")
n = obj:Getname()
-- create queue to hold messages
q = TBL()
--create messages
msg1 = obj:CreateMSG("ee3",{m1 = "hello"})
msg2 = obj:CreateMSG("ee4",{m1 =",world",m2 = "today"})
--send messages "Adds the message to the queue"
obj:SendMSG(msg1)



q:Iter(print(msg2.msgs.m1))
in_use:Add(msg1.id)
if in_use:CheckID(msg1.id) then

print("present")
end
--]]



