//--------------------------------------------------------
// OpenNero : GuiBase
//  a basic container for a gui piece
//--------------------------------------------------------

#include "core/Common.h"
#include "GuiBase.h"
#include "core/Log.h"
#include "gui/GuiManager.h"

namespace OpenNero
{
    std::string wstring_to_string( const std::wstring& str16 )
    {
        // TODO : There has got to be a better (and already written) way to do this...
        std::string str8;
        str8.resize( str16.length() );
        Assert( str8.length() == str16.length() );

        for( uint32_t i = 0; i < str16.length(); ++i )
            str8[i] = (uint8_t)str16[i];

        return str8;
    }



    GuiBase::GuiBase()
        : mFocused(false)
    {}

    GuiBase::~GuiBase()
    {
        if( mpGuiElement && mpGuiElement->getParent() )
        {
            mpGuiElement->remove();
        }
    }

	// Set our visibility state
	void GuiBase::setVisible( bool visible )
	{
		Assert( mpGuiElement );
		mpGuiElement->setVisible(visible);
	}

    bool GuiBase::isVisible()
    {
        Assert( mpGuiElement );
		return mpGuiElement->isVisible();
    }

	// Set our 'enabledness' state
	void GuiBase::setEnabled( bool enabled )
	{
		Assert( mpGuiElement );
		mpGuiElement->setEnabled(enabled);
	}

    bool GuiBase::isEnabled()
    {
        Assert( mpGuiElement );
        return mpGuiElement->isEnabled();
    }

    // Notify the element that we have gained focus
    void GuiBase::setFocus()
    {
        Assert( getIrrGuiElement() );
        Assert( mpManagerProxy );

        mpManagerProxy->SetFocusStatus( getId() ,true);
        mFocused = true;
	}

    // Notify the element that we have lost focus
    void GuiBase::removeFocus()
    {
        Assert( getIrrGuiElement() );
        Assert( mpManagerProxy );

        mpManagerProxy->SetFocusStatus( getId(),false);
        mFocused = false;
    }

    bool GuiBase::isFocused() const
    {
        return mFocused;
    }

	// Get the id of our parent
	int32_t GuiBase::getParentId() const
	{
		Assert( mpGuiElement );
		IGUIElement* parent = mpGuiElement->getParent();
		return ( parent ? parent->getID() : -1 );
	}

    // Get our id
    int32_t GuiBase::getId() const
    {
        Assert( mpGuiElement );
        return mpGuiElement->getID();
    }

    // Get our name
    std::string GuiBase::getName() const
    {
        return mGuiName;
    }

    // Set the name of this element
    // @param name the name to set
    void GuiBase::setName( const std::string& name )
    {
        if(mpManagerProxy)
            mpManagerProxy->ModifyRegisteredElementName( getName(), name );

        mGuiName = name;
    }

	// Add a child to us
	void GuiBase::addChild( GuiBasePtr child )
	{
		Assert( mpGuiElement );
		mpGuiElement->addChild( child->mpGuiElement.get() );
	}

	// Set a textual identifier to register with ourselves
    void GuiBase::setText( const std::wstring& text )
	{
		Assert( mpGuiElement );
        mText = text;
        mpGuiElement->setText( mText.c_str() );
	}

    void GuiBase::setText( const std::string& text )
    {
        // This probably isn't very efficient.
        setText( boost::lexical_cast<std::wstring>( text.c_str() ) );
    }

    std::wstring GuiBase::getText() const
    {
        Assert( mpGuiElement );
        return mpGuiElement->getText();
    }

    std::string  GuiBase::getSText()
    {
        return wstring_to_string( getText() );
    }

    // get the absolute position of the element on screen
    Rect2i GuiBase::getAbsoluteBounds() const
    {
        Assert( mpGuiElement );
        return mpGuiElement->getAbsolutePosition();
    }

    // get the position relative to the parent
    Rect2i GuiBase::getRelativeBounds()
    {
        Assert( mpGuiElement );
        return mpGuiElement->getRelativePosition();
    }

    void GuiBase::setRelativeBounds( const Rect2i& rect )
    {
        Assert( mpGuiElement );
        mElementBounds = rect;
        mpGuiElement->setRelativePosition(rect);
    }

    void GuiBase::setRelativePosition( const Pos2i& pos )
    {
        Assert( mpGuiElement );
        mElementBounds.UpperLeftCorner = pos;
        mpGuiElement->setRelativePosition(mElementBounds);
    }

    Pos2i GuiBase::getRelativePosition()
    {
        return mElementBounds.UpperLeftCorner;
    }

    void GuiBase::setExtent( const Pos2i& ext )
    {
        Assert( mpGuiElement );
        mElementBounds.LowerRightCorner = mElementBounds.UpperLeftCorner + ext;
        mpGuiElement->setRelativePosition(mElementBounds);
    }

    Pos2i GuiBase::getExtent()
    {
        return mElementBounds.LowerRightCorner - mElementBounds.UpperLeftCorner;
    }

	// Recalculate the absolute position after a movement or a heiarchy reorganization
	void GuiBase::updateAbsolutePosition()
	{
		Assert( mpGuiElement );
		mpGuiElement->updateAbsolutePosition();
	}

	// Remove the gui piece from the scene and get rid of reference
    void GuiBase::removeFromScene()
    {
        if( mpGuiElement )
        {
            mpGuiElement->remove();
        }
    }

    void GuiBase::setGuiElement( IGuiElement_IPtr guiElement )
    {
        mpGuiElement = guiElement;
    }

    void GuiBase::SetOnMouseLeftDown( PyObject func )       {   SetResponseFunction( kResponse_OnMouseLeftDown, func); }
    void GuiBase::SetOnMouseRightDown( PyObject func )      {   SetResponseFunction( kResponse_OnMouseRightDown, func); }
    void GuiBase::SetOnMouseMiddleDown( PyObject func )     {   SetResponseFunction( kResponse_OnMouseMiddleDown, func); }
    void GuiBase::SetOnMouseLeftClick( PyObject func )      {   SetResponseFunction( kResponse_OnMouseLeftClick, func); }
    void GuiBase::SetOnMouseRightClick( PyObject func )     {   SetResponseFunction( kResponse_OnMouseRightClick, func); }
    void GuiBase::SetOnMouseMiddleClick( PyObject func )    {   SetResponseFunction( kResponse_OnMouseMiddleClick, func); }
    void GuiBase::SetOnMouseEnter( PyObject func )          {   SetResponseFunction( kResponse_OnMouseEnter, func); }
    void GuiBase::SetOnMouseLeave( PyObject func )          {   SetResponseFunction( kResponse_OnMouseLeave, func); }
    void GuiBase::SetOnMouseMove( PyObject func )           {   SetResponseFunction( kResponse_OnMouseMove, func); }
    void GuiBase::SetOnScrollBarChange( PyObject func )     {   SetResponseFunction( kResponse_OnScrollBarChange, func); }
    void GuiBase::SetOnCheckBoxChange( PyObject func )      {   SetResponseFunction( kResponse_OnCheckBoxChange, func); }

    GuiBase::PyObject GuiBase::GetOnMouseLeftDown()       {   return GetResponseFunction( kResponse_OnMouseLeftDown); }
    GuiBase::PyObject GuiBase::GetOnMouseRightDown()      {   return GetResponseFunction( kResponse_OnMouseRightDown); }
    GuiBase::PyObject GuiBase::GetOnMouseMiddleDown()     {   return GetResponseFunction( kResponse_OnMouseMiddleDown); }
    GuiBase::PyObject GuiBase::GetOnMouseLeftClick()      {   return GetResponseFunction( kResponse_OnMouseLeftClick); }
    GuiBase::PyObject GuiBase::GetOnMouseRightClick()     {   return GetResponseFunction( kResponse_OnMouseRightClick); }
    GuiBase::PyObject GuiBase::GetOnMouseMiddleClick()    {   return GetResponseFunction( kResponse_OnMouseMiddleClick); }
    GuiBase::PyObject GuiBase::GetOnMouseEnter()          {   return GetResponseFunction( kResponse_OnMouseEnter); }
    GuiBase::PyObject GuiBase::GetOnMouseLeave()          {   return GetResponseFunction( kResponse_OnMouseLeave); }
    GuiBase::PyObject GuiBase::GetOnMouseMove()           {   return GetResponseFunction( kResponse_OnMouseMove); }
    GuiBase::PyObject GuiBase::GetOnScrollBarChange()     {   return GetResponseFunction( kResponse_OnScrollBarChange); }
    GuiBase::PyObject GuiBase::GetOnCheckBoxChange()      {   return GetResponseFunction( kResponse_OnCheckBoxChange); }

    void GuiBase::SetResponseFunction( ResponseFlag flag, PyObject func )
    {
        Assert( flag >= (ResponseFlag)0 && flag < kResponse_Max );
        mResponseFunctions[flag] = func;
    }

    GuiBase::PyObject GuiBase::GetResponseFunction( ResponseFlag flag )
    {
        Assert( flag >= (ResponseFlag)0 && flag < kResponse_Max );
        return mResponseFunctions[flag];
    }

    IGuiElement_IPtr GuiBase::getGuiElement()
    {
        return mpGuiElement;
    }

    /**
     * Convert a 2D position in the absolute frame to a 2D position in my relative frame
     * @param pos2D the absolute position
     * @return the relative position
    */
    position2di GuiBase::convertToRelative( const position2di& pos2D )
    {
        Assert( mpGuiElement );

        const core::rect<int32_t> absPos( mpGuiElement->getAbsolutePosition() );
        return position2di( pos2D.X - absPos.UpperLeftCorner.X,
                            pos2D.Y - absPos.UpperLeftCorner.Y );
    }

	/**
	 * Get the Irrlicht gui element we store virtually. Note: this was done to prevent
	 * the upcast cost and danger of simply storing an IGUIElement* and casting to what
	 * type we need. Of course now we have a vtable cost...
	 * @return the irrlicht gui element ptr contained by this class
	*/
	IGUIElement* GuiBase::getIrrGuiElement()
	{
		AssertMsg( mpGuiElement, "Invalid Gui Element!" );
		return mpGuiElement.get();
	}

    void GuiBase::setSimFactory( SimFactoryPtr factory )
    {
        mpSimFactory = factory;
    }

    SimFactoryPtr GuiBase::getSimFactory()
    {
        return mpSimFactory;
    }

    void GuiBase::setManagerProxy( GuiManagerProxyPtr proxy )
    {
        mpManagerProxy = proxy;
    }



    // can this element gain focus of the keyboard?
    bool PyGuiBase::CanGainFocus()
    {
        return Default_CanGainFocus();
    }

    bool PyGuiBase::RespondsTo( ResponseFlag flag )
    {
        return RespondsTo_Internal(flag);
    }

    bool PyGuiBase::OnMouseLeftDown  ( /*const SEvent& event*/ )
    {
        if( !AttemptPythonExecution(kResponse_OnMouseLeftDown) && !AttemptOverrideExecution( "OnMouseLeftDown" ) )
            return Default_OnMouseLeftDown(/*event*/);

        return true;
    }

    bool PyGuiBase::OnMouseRightDown ( /*const SEvent& event*/ )
    {
        if( !AttemptPythonExecution(kResponse_OnMouseRightDown) && !AttemptOverrideExecution( "OnMouseRightDown" ) )
            return Default_OnMouseRightDown(/*event*/);

        return true;
    }
    bool PyGuiBase::OnMouseMiddleDown( /*const SEvent& event*/ )
    {
        if( !AttemptPythonExecution(kResponse_OnMouseMiddleDown) && !AttemptOverrideExecution( "OnMouseMiddleDown" ) )
            return Default_OnMouseMiddleDown(/*event*/);

        return true;
    }

    bool PyGuiBase::OnMouseLeftClick  ( /*const SEvent& event*/ )
    {
        if( !AttemptPythonExecution(kResponse_OnMouseLeftClick) && !AttemptOverrideExecution( "OnMouseLeftClick" ) )
            return Default_OnMouseLeftClick(/*event*/);

        return true;
    }

    bool PyGuiBase::OnMouseRightClick ( /*const SEvent& event*/ )
    {
        if( !AttemptPythonExecution(kResponse_OnMouseRightClick) && !AttemptOverrideExecution( "OnMouseRightClick" ) )
            return Default_OnMouseRightClick(/*event*/);

        return true;
    }

    bool PyGuiBase::OnMouseMiddleClick( /*const SEvent& event*/ )
    {
        if( !AttemptPythonExecution(kResponse_OnMouseMiddleClick) && !AttemptOverrideExecution( "OnMouseMiddleClick" ) )
            return Default_OnMouseMiddleClick(/*event*/);

        return true;
    }

    bool PyGuiBase::OnMouseEnter( /*const SEvent& event*/ )
    {
        if( !AttemptPythonExecution(kResponse_OnMouseEnter) && !AttemptOverrideExecution( "OnMouseEnter" ) )
            return Default_OnMouseEnter(/*event*/);

        return true;
    }

    bool PyGuiBase::OnMouseLeave( /*const SEvent& event*/ )
    {
        if( !AttemptPythonExecution(kResponse_OnMouseLeave) && !AttemptOverrideExecution( "OnMouseLeave" ) )
            return Default_OnMouseLeave(/*event*/);

        return true;
    }

    bool PyGuiBase::OnMouseMove(  /*const SEvent& event*/ )
    {
        if( !AttemptPythonExecution(kResponse_OnMouseMove) && !AttemptOverrideExecution( "OnMouseMove" ) )
            return Default_OnMouseMove(/*event*/);

        return true;
    }

    bool PyGuiBase::OnScrollBarChange(  /*const SEvent& event*/ )
    {
        if( !AttemptPythonExecution(kResponse_OnScrollBarChange) && !AttemptOverrideExecution( "OnScrollBarChange" ) )
            return Default_OnScrollBarChange(/*event*/);

        return true;
    }
    
    bool PyGuiBase::OnCheckBoxChange( /*const SEvent& event*/ )
    {
        if (!AttemptPythonExecution(kResponse_OnCheckBoxChange) && !(AttemptOverrideExecution( "OnCheckBoxChange" )))
            return Default_OnCheckBoxChange(/* event */);
        return true;
    }

    bool PyGuiBase::RespondsTo_Internal( ResponseFlag flag )
    {
        // only respond if we are enabled
        if( isEnabled() )
        {
            // first check to see if we have a python function to execute
            PyObject responseFunc = GetResponseFunction(flag);
            if( responseFunc.ptr() != Py_None )
                return true;

            // otherwise check for an override in a child python class
            switch( flag )
            {
            case kResponse_OnMouseLeftDown:     return CheckForOverride("OnMouseLeftDown");
            case kResponse_OnMouseRightDown:    return CheckForOverride("OnMouseRightDown");
            case kResponse_OnMouseMiddleDown:   return CheckForOverride("OnMouseMiddleDown");

            case kResponse_OnMouseLeftClick:    return CheckForOverride("OnMouseLeftClick");
            case kResponse_OnMouseRightClick:   return CheckForOverride("OnMouseRightClick");
            case kResponse_OnMouseMiddleClick:  return CheckForOverride("OnMouseMiddleClick");

            case kResponse_OnMouseEnter:        return CheckForOverride("OnMouseEnter");
            case kResponse_OnMouseLeave:        return CheckForOverride("OnMouseLeave");
            case kResponse_OnMouseMove:         return CheckForOverride("OnMouseMove");

            case kResponse_OnScrollBarChange:   return CheckForOverride("OnScrollBarChange");
            
            case kResponse_OnCheckBoxChange:    return CheckForOverride("OnCheckBoxChange");

            default: Assert(false); return false;
            }
        }

        return false;
    }

    bool PyGuiBase::Default_CanGainFocus() { return false; }
    bool PyGuiBase::Default_OnMouseLeftDown  ( /*const SEvent& event*/ ) { return false; }
    bool PyGuiBase::Default_OnMouseRightDown ( /*const SEvent& event*/ ) { return false; }
    bool PyGuiBase::Default_OnMouseMiddleDown( /*const SEvent& event*/ ) { return false; }

    bool PyGuiBase::Default_OnMouseLeftClick  ( /*const SEvent& event*/ ) { return false; }
    bool PyGuiBase::Default_OnMouseRightClick ( /*const SEvent& event*/ ) { return false; }
    bool PyGuiBase::Default_OnMouseMiddleClick( /*const SEvent& event*/ ) { return false; }

    bool PyGuiBase::Default_OnMouseEnter( /*const SEvent& event*/ ) { return false; }
    bool PyGuiBase::Default_OnMouseLeave( /*const SEvent& event*/ ) { return false; }
    bool PyGuiBase::Default_OnMouseMove(  /*const SEvent& event*/ ) { return false; }

    bool PyGuiBase::Default_OnScrollBarChange() { return false; }
    bool PyGuiBase::Default_OnCheckBoxChange() { return false; }

    bool PyGuiBase::CheckForOverride( const std::string& overrideName ) const
    {
        return static_cast<bool>(this->get_override(overrideName.c_str()));
    }

    bool PyGuiBase::AttemptOverrideExecution( const std::string& overrideName )
    {
        using namespace boost::python;

        // get the override object to execute
        override func = this->get_override(overrideName.c_str());
        if( func )
        {
            // attempt to execute the override
            TryCall<override>(func);

            // we found a valid override, return true
            return true;
        }

        // did not find a valid override, return false
        return false;
    }

    bool PyGuiBase::AttemptPythonExecution( ResponseFlag flag )
    {
        GuiBase::PyObject pyObj = GetResponseFunction(flag);
        if( pyObj.ptr() != Py_None )
        {
            // attempt to execute the python object function
            TryCall<GuiBase::PyObject>(pyObj);
            return true;
        }

        return false;
    }

} // end OpenNero
