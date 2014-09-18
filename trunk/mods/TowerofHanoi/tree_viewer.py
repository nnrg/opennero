import Tkinter as tk
import time
import threading
import Queue

class ViewerItem:
    """
    Data structure to hold a list of planning elements to visualize.  Uses could include showing a list of possible actions, an action's preconditions, etc.  The tk_widget object that is passed to this class is it's drawing space, the following widget types are currently supported: ----------------------  
    """
    def __init__(self, tk_frame, title, items, active_index = -1, hidden_index = [], completed_index = [], show_hidden_as_grey=True, display_type="text_column"):
        
        self.items = items
        self.active_index = active_index
        self.hidden_index = list(hidden_index)
        self.completed_index = list(completed_index)
        self.tk_frame = tk_frame
        self.show_hidden_as_grey = show_hidden_as_grey
        #change the background color for the frame so that it will show up better 
        self.tk_frame.config(bg="linen", borderwidth=1.5, relief=tk.SUNKEN)
        ##initiailize the frame
        if (display_type == "text_column"):
            self.number_of_grid_rows = 0
            self.item_views = []
            self.diplayed_item_idecies = []
            self.title = title
            self.item_group_title = tk.Frame(self.tk_frame)
            self.item_group_title_text = tk.Label(self.item_group_title)            
            self.item_group_title_text.config(text=self.title, font=("Helvetica", 11, "bold"), bg="linen")
            self.item_group_title_text.pack(side=tk.TOP)
            self.item_group_title.grid(row=0, column=0)
            #create a display label for each item, and put them all in an array.  Most updates to the display should be able to work off of this array instead of the raw items list
            for item in range(len(self.items)):
                #try not to output unnnessecary items
                if not(items[item][0:7] == "Smaller"):
                    self.item_views.append(tk.Label(self.tk_frame))#str(items[item])))
                    self.item_views[self.number_of_grid_rows].config(text=self.items[item], font=("Helvetica", 9), width=21)
                    self.item_views[self.number_of_grid_rows].grid(row=self.number_of_grid_rows+1, column=0)
                    self.diplayed_item_idecies.append(item)
                    self.number_of_grid_rows += 1
            #now that all the labels have been created handle the active/hidden/completed color stuff
            self.updateWidget()
        else:
            print "specified display type of \"{}\" is not a recognized type, cannot display items".format(display_type)
    def updateWidget(self):
        """ Updates the information shown in the tk widget """
        for grid_row in range(self.number_of_grid_rows):
            #determine what the background color should be (plain/hidden/highlight)
            try:
                self.hidden_index.index(self.diplayed_item_idecies[grid_row]) #if this doesn't crash then this item should be "hidden"
                #the item index is in the list of hidden items, set the background to gray
                self.item_views[grid_row].config(fg="grey60", bg="linen")
            except: #the item index was not in the list of hidden items, see if it is completed or not
                try:
                    self.completed_index.index(self.diplayed_item_idecies[grid_row])  #if this doesn't crash then this item should be "completed"
                    self.item_views[grid_row].config(fg="black", bg="PaleGreen")
                except:  #the item index was not in the list of completed items, see if it is active or not
                    if (self.diplayed_item_idecies[grid_row] == self.active_index): #if this is the active item highlight it
                        self.item_views[grid_row].config(fg="black", bg="yellow")
                    else:
                        self.item_views[grid_row].config(fg="black", bg="linen")            
        
    def setActiveIndex(self, new_active_index):
        """sets the active index (i.e. the active item) to the given value and updates the drawing"""
        self.active_index = new_active_index
        self.updateWidget()
    def addHiddenIndex(self, index_to_add):
        """adds the given item index to the list of items to hide (or grey out) then updates the display.  Makes sure the index isn't alreay there first"""
        try:
            self.hidden_index.index(index_to_add)
        except:
            self.hidden_index.append(index_to_add)
            self.updateWidget()
    def removeHiddenIndex(self, index_to_remove):
        """Removes the given index from the list of hidden indecies, then updates the display"""
        try:
            self.hidden_index.remove(index_to_remove)
        except:
            print "Attemped to remove a visualization hidden item that does not exist, doing nothing."
        self.updateWidget()
    def addCompletedIndex(self, index_to_add):
        """adds the given item index to the list of items to mark completed, then updates the display.  Makes sure the index isn't alreay there first"""
        try:
            self.completed_index.index(index_to_add)
        except:
            self.completed_index.append(index_to_add)
            self.updateWidget()
    def removeCompletedIndex(self, index_to_remove):
        """Removes the given index from the list of completed indecies, then updates the display"""
        try:
            self.completed_index.remove(index_to_remove)
        except:
            print "Attemped to remove a visualization hidden item ({}) that does not exist, doing nothing.".format(index_to_remove)
        self.updateWidget()
    def addItem(self, new_item, hidden=False, completed=False):
        """adds an item to the item list, as long as it doesn't make the item list too long (this is a failsafe to prevent infinite loops from taking more and more memory).  This function also checks to see if the item is in the list already, and if so removes it and updates all index info accordingly"""
        #make sure we are not trying to display a "Smaller" item
        if (not(new_item[0:7] == "Smaller") and len(self.items) < 50):        
            #first deal with removing the old copy of this item (if need be)
            if(self.items.count(new_item)):
                old_instance_index = self.items.index(new_item)
                print "Old isntace of \"{}\" found at index {}".format(new_item, old_instance_index)
                #handle the active item index if it is >= old_instance_index
                if (self.active_index == old_instance_index):
                    self.active_index = -1
                elif (self.active_index > old_instance_index):
                    self.active_index -= 1
                #now deal with the hidden indecies
                if (self.hidden_index.count(old_instance_index)):
                    self.hidden_index.remove(old_instance_index)
                for index in range(len(self.hidden_index)):
                    if (self.hidden_index[index] > old_instance_index):
                        self.hidden_index[index] -=1
                #and then the completed indecies
                if (self.completed_index.count(old_instance_index)):
                    self.completed_index.remove(old_instance_index)
                for index in range(len(self.completed_index)):
                    if (self.completed_index[index] > old_instance_index):
                        self.completed_index[index] -=1
                #now finally get rid of the old instance of the item
                self.items.remove(new_item)
            
            #remove all labels, try to do this without creating a memory leak
            for view in range(len(self.item_views)):
                old = self.item_views[view]
                self.item_views[view] = 1
                del old
            del self.item_views
            del self.diplayed_item_idecies                 
            self.diplayed_item_idecies = []
            self.number_of_grid_rows = 0
            self.item_views = []            
            
            #now add the new item back in
            self.items.append(new_item)
            #now make sure to not if the new item is hidden or completed
            if (hidden):
                self.addHiddenIndex(len(self.items)-1)
            if (completed):
                self.addCompletedIndex(len(self.items)-1)
                

            #recreate the labels
            #create a display label for each item, and put them all in an array.  Most updates to the display should be able to work off of this array instead of the raw items list
            for item in range(len(self.items)):
                #try not to output unnnessecary items
                if not(self.items[item][0:7] == "Smaller"):
                    self.item_views.append(tk.Label(self.tk_frame))#str(items[item])))
                    self.item_views[self.number_of_grid_rows].config(text=self.items[item], font=("Helvetica", 9), width=21)
                    self.item_views[self.number_of_grid_rows].grid(row=self.number_of_grid_rows+1, column=0)
                    self.diplayed_item_idecies.append(item)
                    self.number_of_grid_rows += 1

            #finally, update the GUI
            self.updateWidget()
        else:
            print "Cannot display more items"
            
class TreeViewer(tk.Frame):
    def __init__(self, master, **options):
        tk.Frame.__init__(self, master, **options)
        self.master = master

        self.number_item_viewers = 0
        self.MAX_ITEM_VIEWERS = 10
        self.item_viewers = []
        self.fTop = tk.Frame(self)

        self.text = tk.Text(self.fTop)
        
        self.text.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)

        self.scroll = tk.Scrollbar(self.fTop)
        self.scroll.pack(side=tk.RIGHT, fill=tk.Y)

        self.text.config(font="Courier 12", yscrollcommand=self.scroll.set)
        self.scroll.config(command=self.text.yview)
        
        self.fTop.pack(side=tk.TOP, fill=tk.BOTH, expand=True)

        self.fBottom = tk.Frame(self)

        self.return_button = tk.Button(self.fBottom, text='Execute Plan')
        self.return_button.config(command=self.return_results)
        self.return_button.pack(side=tk.RIGHT)

        self.continue_button = tk.Button(self.fBottom, text='Skip Rest')
        self.continue_button.config(command=self.user_continue)
        self.continue_button.pack(side=tk.RIGHT)

        self.next_button = tk.Button(self.fBottom, text='Next Step')
        self.next_button.config(command=self.user_unpause)
        self.next_button.pack(side=tk.RIGHT)

        self.fBottom.pack(side=tk.TOP)

        self.fPlan = tk.Frame(self)
        self.fPlan.pack(side=tk.BOTTOM)  
        
        self.paused = tk.BooleanVar()
        self.paused.set(False)

        self.continued = tk.BooleanVar()
        self.continued.set(False)

        self.bind('<<display-text>>', self.display_text_handler)
        self.bind('<<user-pause>>', self.user_pause_handler)
        
        self.message = Queue.Queue()
        self.callback = Queue.Queue() # wait on items in this queue to continue
        
        self.pack(fill=tk.BOTH, expand=True)

        def window_close_handler():
            self.return_results()

        self.master.protocol("WM_DELETE_WINDOW", window_close_handler)

    def add_item_viewer(self, title, items, active_index = -1, hidden_index = [], completed_index = [], show_hidden_as_grey=True, display_type="text_column"):
        if (self.number_item_viewers < self.MAX_ITEM_VIEWERS):
            self.item_viewers.append(ViewerItem(tk.Frame(self.fPlan), title, items, active_index, hidden_index, completed_index, 
show_hidden_as_grey, display_type))
            self.item_viewers[self.number_item_viewers].tk_frame.pack(side=tk.RIGHT)
            self.number_item_viewers += 1
        #else:
         #   print "The current limit on visible item viewers is {}, no new items will be created until some are removed".format(self.MAX_ITEM_VIEWERS)

    def set_active_index(self, new_active_index, viewer_index):
        """sets the active index (i.e. the active item) to the given value and updates the drawing"""
        if (viewer_index < self.MAX_ITEM_VIEWERS):
            self.item_viewers[viewer_index].setActiveIndex(new_active_index)

    def add_hidden_index(self, index_to_add, viewer_index):
        """adds the given item index to the list of items to hide (or grey out) then updates the display.  Makes sure the index isn't alreay there first"""
        if (viewer_index < self.MAX_ITEM_VIEWERS):
            self.item_viewers[viewer_index].addHiddenIndex(index_to_add)

    def remove_hidden_index(self, index_to_remove, viewer_index):
        """Removes the given index from the list of hidden indecies, then updates the display"""
        if (viewer_index < self.MAX_ITEM_VIEWERS):
            self.item_viewers[viewer_index].removeHiddenIndex(index_to_remove)

    def add_completed_index(self, index_to_add, viewer_index):
        """adds the given item index to the list of items to mark completed, then updates the display.  Makes sure the index isn't alreay there first"""
        if (viewer_index < self.MAX_ITEM_VIEWERS):
            self.item_viewers[viewer_index].addCompletedIndex(index_to_add)

    def remove_completed_index(self, index_to_remove, viewer_index):
        """Removes the given index from the list of completed indecies, then updates the display"""
        if (viewer_index < self.MAX_ITEM_VIEWERS):
            self.item_viewers[viewer_index].removeCompletedIndex(index_to_remove)
        
    def add_item_to_viewer(self, item, viewer_index, hidden=False, completed=False):
        """adds an item to the item list, as long as it doesn't make the item list too long (this is a failsafe to prevent infinite loops from taking more and more memory).  This function also checks to see if the item is in the list already, and if so removes it and updates all index info accordingly"""
        if (viewer_index < self.MAX_ITEM_VIEWERS):
            self.item_viewers[viewer_index].addItem(item, hidden, completed)
    
    def remove_last_item_viewer(self, viewer_index=-1):
        """viewer index can be used as a sanity check"""
        if (viewer_index == len(self.item_viewers) - 1) or (viewer_index == -1):
            last_viewer = self.item_viewers.pop()
            last_viewer.tk_frame.destroy()
            del last_viewer
            self.number_item_viewers -= 1
        else:
            print "WARNING [TreeViewer.remove_last_item_viewer]: viewer_index {} is not the last viewer widget in the array".format(viewer_index)

    def display_text(self, s):
        self.message.put(s)
        self.event_generate('<<display-text>>')

    def display_text_handler(self, event=None):
        s = self.message.get()
        self.text.insert(tk.END, s)
        self.text.insert(tk.END, '\n')
        self.text.yview(tk.END)

    def user_pause(self, s):
        self.display_text(s)
        self.event_generate('<<user-pause>>')
        # blocks until the callback queue is filled
        self.callback.get()

    def user_pause_handler(self, event=None):
        if self.continued.get():
            self.callback.put('go ahead')
        # now check back every now and then until 
        self.paused.set(True)
        self.after(100, self.check_if_unpaused)

    def check_if_unpaused(self, event=None):
        if self.continued.get() or not self.paused.get():
            self.callback.put('go ahead 2')
        else:
            self.after(100, self.check_if_unpaused)
 
    def user_unpause(self):
        self.paused.set(False)

    def user_continue(self):
        self.paused.set(False)
        self.continued.set(True)

    def return_results(self):
        if self.continued.get() or not self.paused.get():
            self.master.quit()
        else:
            # Viewer is paused. Continuing before returning...
            self.user_continue()
            self.master.after(1000, self.master.quit)

if __name__ == "__main__":
    viewer = TreeViewer()
    tk.mainloop()
