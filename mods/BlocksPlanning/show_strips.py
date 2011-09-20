import wx

"""
Displays the STRIPS action plan as a wxPython TreeCtrl
"""

class StripsFrame(wx.Frame):
    def __init__(self, parent, id, title):
        wx.Frame.__init__(self, parent, id, title, wx.DefaultPosition, wx.Size(450, 350))

        hbox = wx.BoxSizer(wx.HORIZONTAL)
        vbox = wx.BoxSizer(wx.VERTICAL)
        panel1 = wx.Panel(self, -1)
        panel2 = wx.Panel(self, -1)
        self.tree = wx.TreeCtrl(panel1, 1, wx.DefaultPosition, (-1,-1), wx.TR_HIDE_ROOT|wx.TR_HAS_BUTTONS)
        self.tree.Bind(wx.EVT_TREE_SEL_CHANGED, self.OnSelChanged, id=1)
        self.display = wx.StaticText(panel2, -1, '',(10,10), style=wx.ALIGN_CENTRE)
        vbox.Add(self.tree, 1, wx.EXPAND)
        hbox.Add(panel1, 1, wx.EXPAND)
        hbox.Add(panel2, 1, wx.EXPAND)
        panel1.SetSizer(vbox)
        self.SetSizer(hbox)
        self.Centre()

    def ShowSolution(self, solution):
        actions = reversed([x for x in solution])
        root = self.tree.AddRoot('Solution Plan')
        for i, action in enumerate(actions):
            name = action.simple_str()
            item = self.tree.AppendItem(root, '%i) - %s' % (i,name))
            pre_tree = self.tree.AppendItem(item, 'Pre-conditions')
            for pre in [str(s) for s in action.pre]:
                self.tree.AppendItem(pre_tree, pre)
            post_tree = self.tree.AppendItem(item, 'Post-conditions')
            for post in [str(s) for s in action.post]:
                self.tree.AppendItem(post_tree, post)

    def OnSelChanged(self, event):
        item =  event.GetItem()
        self.display.SetLabel(self.tree.GetItemText(item))

class StripsApp(wx.App):
    def OnInit(self):
        self.frame = StripsFrame(None, -1, 'STRIPS Solution Plan')
        self.frame.Show(True)
        self.SetTopWindow(self.frame)
        return True
    def ShowSolution(self, solution):
        self.frame.ShowSolution(solution)

def show_solution(solution):
    app = StripsApp(0)
    app.ShowSolution(solution)
    app.MainLoop()
