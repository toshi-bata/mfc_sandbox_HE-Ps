#include "stdafx.h"
#include "CHPSApp.h"
#include "CHPSDoc.h"
#include "CHPSView.h"
#include "sprk.h"
#include "SandboxHighlightOp.h"
#include "CProgressDialog.h"
#include <WinUser.h>

#include "CreateSolidDlg.h"
#include "BlendDlg.h"
#include "DeleteFaceDlg.h"
//#include "BooleanOp.h"
#include "ClickEntitiesCmdOp.h"
#include "HollowDlg.h"
#include "DeleteCompDlg.h"
#include "FeatureRecognitionDlg.h"
#include "BooleanDlg.h"
#include "MirrorBodyDlg.h"

#ifdef USING_PUBLISH
#include "sprk_publish.h"
#endif


using namespace HPS;

IMPLEMENT_DYNCREATE(CHPSView, CView)

BEGIN_MESSAGE_MAP(CHPSView, CView)
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_RBUTTONDBLCLK()
	ON_WM_MBUTTONDOWN()
	ON_WM_MBUTTONUP()
	ON_WM_MBUTTONDBLCLK()
	ON_WM_MOUSEHWHEEL()
	ON_WM_MOUSEWHEEL()
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	ON_WM_KILLFOCUS()
	ON_WM_SETFOCUS()
	ON_COMMAND(ID_EDIT_COPY, &CHPSView::OnEditCopy)
	ON_COMMAND(ID_FILE_OPEN, &CHPSView::OnFileOpen)
	ON_COMMAND(ID_FILE_SAVE_AS, &CHPSView::OnFileSaveAs)
	ON_COMMAND(ID_OPERATORS_ORBIT, &CHPSView::OnOperatorsOrbit)
	ON_COMMAND(ID_OPERATORS_PAN, &CHPSView::OnOperatorsPan)
	ON_COMMAND(ID_OPERATORS_ZOOM_AREA, &CHPSView::OnOperatorsZoomArea)
	ON_COMMAND(ID_OPERATORS_FLY, &CHPSView::OnOperatorsFly)
	ON_COMMAND(ID_OPERATORS_HOME, &CHPSView::OnOperatorsHome)
	ON_COMMAND(ID_OPERATORS_ZOOM_FIT, &CHPSView::OnOperatorsZoomFit)
	ON_COMMAND(ID_OPERATORS_SELECT_POINT, &CHPSView::OnOperatorsSelectPoint)
	ON_COMMAND(ID_OPERATORS_SELECT_AREA, &CHPSView::OnOperatorsSelectArea)
	ON_COMMAND(ID_MODES_SIMPLE_SHADOW, &CHPSView::OnModesSimpleShadow)
	ON_COMMAND(ID_MODES_SMOOTH, &CHPSView::OnModesSmooth)
	ON_COMMAND(ID_MODES_HIDDEN_LINE, &CHPSView::OnModesHiddenLine)
	ON_COMMAND(ID_MODES_EYE_DOME_LIGHTING, &CHPSView::OnModesEyeDome)
	ON_COMMAND(ID_MODES_FRAME_RATE, &CHPSView::OnModesFrameRate)
	ON_COMMAND(ID_USER_CODE_1, &CHPSView::OnUserCode1)
	ON_COMMAND(ID_USER_CODE_2, &CHPSView::OnUserCode2)
	ON_COMMAND(ID_USER_CODE_3, &CHPSView::OnUserCode3)
	ON_COMMAND(ID_USER_CODE_4, &CHPSView::OnUserCode4)
	ON_WM_MOUSELEAVE()
	ON_UPDATE_COMMAND_UI(ID_MODES_SIMPLE_SHADOW, &CHPSView::OnUpdateRibbonBtnSimpleShadow)
	ON_UPDATE_COMMAND_UI(ID_MODES_FRAME_RATE, &CHPSView::OnUpdateRibbonBtnFrameRate)
	ON_UPDATE_COMMAND_UI(ID_MODES_SMOOTH, &CHPSView::OnUpdateRibbonBtnSmooth)
	ON_UPDATE_COMMAND_UI(ID_MODES_HIDDEN_LINE, &CHPSView::OnUpdateRibbonBtnHiddenLine)
	ON_UPDATE_COMMAND_UI(ID_MODES_EYE_DOME_LIGHTING, &CHPSView::OnUpdateRibbonBtnEyeDome)
	ON_UPDATE_COMMAND_UI(ID_OPERATORS_ORBIT, &CHPSView::OnUpdateRibbonBtnOrbitOp)
	ON_UPDATE_COMMAND_UI(ID_OPERATORS_PAN, &CHPSView::OnUpdateRibbonBtnPanOp)
	ON_UPDATE_COMMAND_UI(ID_OPERATORS_ZOOM_AREA, &CHPSView::OnUpdateRibbonBtnZoomAreaOp)
	ON_UPDATE_COMMAND_UI(ID_OPERATORS_FLY, &CHPSView::OnUpdateRibbonBtnFlyOp)
	ON_UPDATE_COMMAND_UI(ID_OPERATORS_SELECT_POINT, &CHPSView::OnUpdateRibbonBtnPointOp)
	ON_UPDATE_COMMAND_UI(ID_OPERATORS_SELECT_AREA, &CHPSView::OnUpdateRibbonBtnAreaOp)
	ON_COMMAND(ID_BUTTON_BOOL, &CHPSView::OnButtonBool)
	ON_COMMAND(ID_BUTTON_FR, &CHPSView::OnButtonFr)
	ON_COMMAND(ID_BUTTON_DELETE_FACE, &CHPSView::OnButtonDeleteFace)
	ON_COMMAND(ID_BUTTON_HOLLOW, &CHPSView::OnButtonHollow)
	ON_COMMAND(ID_BUTTON_DELETE_BODY, &CHPSView::OnButtonDeleteBody)
	ON_COMMAND(ID_BUTTON_MIRROR, &CHPSView::OnButtonMirror)
END_MESSAGE_MAP()

CHPSView::CHPSView()
	: _enableSimpleShadows(false),
	_enableFrameRate(false),
	_displayResourceMonitor(false),
	_capsLockState(false),
	_eyeDome(false)
{
	_operatorStates[SandboxOperators::OrbitOperator] = true;
	_operatorStates[SandboxOperators::PanOperator] = false;
	_operatorStates[SandboxOperators::ZoomAreaOperator] = false;
	_operatorStates[SandboxOperators::PointOperator] = false;
	_operatorStates[SandboxOperators::AreaOperator] = false;

	_capsLockState = IsCapsLockOn();

#ifdef USING_EXCHANGE_PARASOLID
	m_pProcess = new ExPsProcess();
	m_psMapper = NULL;
#else
	m_pProcess = new ExProcess();
#endif
}

CHPSView::~CHPSView()
{
	_canvas.Delete();

	delete m_pProcess;
}

BOOL CHPSView::PreCreateWindow(CREATESTRUCT& cs)
{
	// Setup Window class to work with HPS rendering.  The REDRAW flags prevent
	// flickering when resizing, and OWNDC allocates a single device context to for this window.
	cs.lpszClass = AfxRegisterWndClass(CS_OWNDC|CS_DBLCLKS|CS_HREDRAW|CS_VREDRAW);
	return CView::PreCreateWindow(cs);
}

static bool has_called_initial_update = false;

void CHPSView::OnInitialUpdate()
{
	has_called_initial_update = true;

	// Only perform HPS::Canvas initialization once since we're reusing the same MFC Window each time.
	if (_canvas.Type() == HPS::Type::None)
	{
		//! [window_handle]
		// Setup to use the DX11 driver
		HPS::ApplicationWindowOptionsKit		windowOpts;
		windowOpts.SetDriver(HPS::Window::Driver::Default3D);

		// Create our Sprockets Canvas with the specified driver
		_canvas = HPS::Factory::CreateCanvas(reinterpret_cast<HPS::WindowHandle>(m_hWnd), "mfc_sandbox_canvas", windowOpts);
		//! [window_handle]
		
		//! [attach_view]
		// Create a new Sprockets View and attach it to our Sprockets.Canvas
		HPS::View view = HPS::Factory::CreateView("mfc_sandbox_view");
		_canvas.AttachViewAsLayout(view);
		//! [attach_view]

		// Setup scene startup values
		SetupSceneDefaults();
	}

	UpdateEyeDome(false);

	_canvas.Update(HPS::Window::UpdateType::Refresh);

	HPS::Rendering::Mode mode = GetCanvas().GetFrontView().GetRenderingMode();
	UpdateRenderingMode(mode);
}

void CHPSView::UpdateEyeDome(bool update)
{
	HPS::WindowKey window = _canvas.GetWindowKey();
	window.GetPostProcessEffectsControl().SetEyeDomeLighting(_eyeDome);

	auto visual_effects_control = window.GetVisualEffectsControl();
	visual_effects_control.SetEyeDomeLightingEnabled(_eyeDome);

	if (update)
		_canvas.Update(HPS::Window::UpdateType::Refresh);
}

void CHPSView::OnPaint()
{
	//! [on_paint]
	// Update our HPS Canvas.  A refresh is needed as the window size may have changed.
	if (has_called_initial_update)
		_canvas.Update(HPS::Window::UpdateType::Refresh);
	//! [on_paint]

	// Invoke BeginPaint/EndPaint.  This must be called when providing OnPaint handler.
	CPaintDC dc(this);
}

void CHPSView::SetMainDistantLight(HPS::Vector const & lightDirection)
{
	HPS::DistantLightKit	light;
	light.SetDirection(lightDirection);
	light.SetCameraRelative(true);
	SetMainDistantLight(light);
}

void CHPSView::SetMainDistantLight(HPS::DistantLightKit const & light)
{
	// Delete previous light before inserting new one
	if (_mainDistantLight.Type() != HPS::Type::None)
		_mainDistantLight.Delete();
	_mainDistantLight = GetCanvas().GetFrontView().GetSegmentKey().InsertDistantLight(light);
}

void CHPSView::SetupSceneDefaults()
{
	// Attach CHPSDoc created model
	GetCanvas().GetFrontView().AttachModel(GetDocument()->GetModel());

	// Set default operators.
	SetupDefaultOperators();

	// Subscribe _errorHandler to handle errors
	HPS::Database::GetEventDispatcher().Subscribe(_errorHandler, HPS::Object::ClassID<HPS::ErrorEvent>());

	// Subscribe _warningHandler to handle warnings
	HPS::Database::GetEventDispatcher().Subscribe(_warningHandler, HPS::Object::ClassID<HPS::WarningEvent>());

	SetMainDistantLight();

	// AxisTriad and NaviCube
	View view = GetCanvas().GetFrontView();
	view.GetAxisTriadControl().SetVisibility(true).SetInteractivity(true);
	view.GetNavigationCubeControl().SetVisibility(true).SetInteractivity(true);

	GetCanvas().GetFrontView().GetSegmentKey().GetVisibilityControl().SetLines(true);

	GetCanvas().GetFrontView().GetSegmentKey().GetCameraControl().SetProjection(HPS::Camera::Projection::Orthographic);
	
	GetCanvas().Update();
}

//! [setup_operators]
void CHPSView::SetupDefaultOperators()
{
	// Orbit is on top and will be replaced when the operator is changed
	GetCanvas().GetFrontView().GetOperatorControl()
		.Push(new HPS::MouseWheelOperator(), Operator::Priority::Low)
		.Push(new HPS::ZoomOperator(MouseButtons::ButtonMiddle()))
		.Push(new HPS::PanOperator(MouseButtons::ButtonRight()))
		.Push(new HPS::OrbitOperator(MouseButtons::ButtonLeft()))
		.Push(new HPS::NavigationCubeOperator(MouseButtons::ButtonLeft()));
}
//! [setup_operators]

void CHPSView::ActivateCapture(HPS::Capture & capture)
{
	HPS::View newView = capture.Activate();
	auto newViewSegment = newView.GetSegmentKey();
	HPS::CameraKit newCamera;
	newViewSegment.ShowCamera(newCamera);

	newCamera.UnsetNearLimit();
	auto defaultCameraWithoutNearLimit = HPS::CameraKit::GetDefault().UnsetNearLimit();
	if (newCamera == defaultCameraWithoutNearLimit)
	{
		HPS::View oldView = GetCanvas().GetFrontView();
		HPS::CameraKit oldCamera;
		oldView.GetSegmentKey().ShowCamera(oldCamera);

		newViewSegment.SetCamera(oldCamera);
		newView.FitWorld();
	}

	AttachViewWithSmoothTransition(newView);
}

#ifdef USING_EXCHANGE
void CHPSView::ActivateCapture(HPS::ComponentPath & capture_path)
{
	HPS::Exchange::Capture capture = (HPS::Exchange::Capture)(capture_path.Front());
	HPS::View newView = capture.Activate(capture_path);
	auto newViewSegment = newView.GetSegmentKey();
	HPS::CameraKit newCamera;
	newViewSegment.ShowCamera(newCamera);

	newCamera.UnsetNearLimit();
	auto defaultCameraWithoutNearLimit = HPS::CameraKit::GetDefault().UnsetNearLimit();
	if (newCamera == defaultCameraWithoutNearLimit)
	{
		HPS::View oldView = GetCanvas().GetFrontView();
		HPS::CameraKit oldCamera;
		oldView.GetSegmentKey().ShowCamera(oldCamera);

		newViewSegment.SetCamera(oldCamera);
		newView.FitWorld();
	}

	AttachViewWithSmoothTransition(newView);
}
#endif

void CHPSView::AttachView(HPS::View & newView)
{
	HPS::CADModel cadModel = GetCHPSDoc()->GetCADModel();
	if (!cadModel.Empty())
	{
		cadModel.ResetVisibility(_canvas);
		_canvas.GetWindowKey().GetHighlightControl().UnhighlightEverything();
	}

	_preZoomToKeyPathCamera.Reset();
	_zoomToKeyPath.Reset();

	HPS::View oldView = GetCanvas().GetFrontView();

	GetCanvas().AttachViewAsLayout(newView);

	HPS::OperatorPtrArray operators;
	auto oldViewOperatorCtrl = oldView.GetOperatorControl();
	auto newViewOperatorCtrl = newView.GetOperatorControl();
	oldViewOperatorCtrl.Show(HPS::Operator::Priority::Low, operators);
	newViewOperatorCtrl.Set(operators, HPS::Operator::Priority::Low);
	oldViewOperatorCtrl.Show(HPS::Operator::Priority::Default, operators);
	newViewOperatorCtrl.Set(operators, HPS::Operator::Priority::Default);
	oldViewOperatorCtrl.Show(HPS::Operator::Priority::High, operators);
	newViewOperatorCtrl.Set(operators, HPS::Operator::Priority::High);

	SetMainDistantLight();

	oldView.Delete();
}

void CHPSView::AttachViewWithSmoothTransition(HPS::View & newView)
{
	HPS::View oldView = GetCanvas().GetFrontView();
	HPS::CameraKit oldCamera;
	oldView.GetSegmentKey().ShowCamera(oldCamera);

	auto newViewSegment = newView.GetSegmentKey();
	HPS::CameraKit newCamera;
	newViewSegment.ShowCamera(newCamera);

	AttachView(newView);

	newViewSegment.SetCamera(oldCamera);
	newView.SmoothTransition(newCamera);
}

void CHPSView::Update()
{
	_canvas.Update();
}

void CHPSView::Unhighlight()
{
	HPS::HighlightOptionsKit highlightOptions;
	highlightOptions.SetStyleName("highlight_style").SetNotification(true);

	_canvas.GetWindowKey().GetHighlightControl().Unhighlight(highlightOptions);
	HPS::Database::GetEventDispatcher().InjectEvent(HPS::HighlightEvent(HPS::HighlightEvent::Action::Unhighlight, HPS::SelectionResults(), highlightOptions));
	HPS::Database::GetEventDispatcher().InjectEvent(HPS::ComponentHighlightEvent(HPS::ComponentHighlightEvent::Action::Unhighlight, GetCanvas(), 0, HPS::ComponentPath(), highlightOptions));
}

void CHPSView::UnhighlightAndUpdate()
{
	Unhighlight();
	Update();
}

void CHPSView::ZoomToKeyPath(HPS::KeyPath const & keyPath)
{
	HPS::BoundingKit bounding;
	if (keyPath.ShowNetBounding(true, bounding))
	{
		_zoomToKeyPath = keyPath;

		HPS::View frontView = _canvas.GetFrontView();
		frontView.GetSegmentKey().ShowCamera(_preZoomToKeyPathCamera);

		HPS::CameraKit fittedCamera;
		frontView.ComputeFitWorldCamera(bounding, fittedCamera);
		frontView.SmoothTransition(fittedCamera);
	}
}

void CHPSView::RestoreCamera()
{
	if (!_preZoomToKeyPathCamera.Empty())
	{
		_canvas.GetFrontView().SmoothTransition(_preZoomToKeyPathCamera);
		HPS::Database::Sleep(500);

		InvalidateZoomKeyPath();
		InvalidateSavedCamera();
	}
}

void CHPSView::InvalidateZoomKeyPath()
{
	_zoomToKeyPath.Reset();
}

void CHPSView::InvalidateSavedCamera()
{
	_preZoomToKeyPathCamera.Reset();
}

HPS::ModifierKeys CHPSView::MapModifierKeys(UINT flags)
{
	HPS::ModifierKeys	modifier;

	// Map shift and control modifiers to HPS::InputEvent modifiers
	if ((flags & MK_SHIFT) != 0)
	{
		if (GetAsyncKeyState(VK_LSHIFT) & 0x8000)
			modifier.LeftShift(true);
		else
			modifier.RightShift(true);
	}

	if ((flags & MK_CONTROL) != 0)
	{
		if (GetAsyncKeyState(VK_LCONTROL) & 0x8000)
			modifier.LeftControl(true);
		else
			modifier.RightControl(true);
	}

	if (_capsLockState)
		modifier.CapsLock(true);

	return modifier;
}

HPS::MatrixKit const & CHPSView::GetPixelToWindowMatrix()
{
	CRect currentWindowRect;
	GetWindowRect(&currentWindowRect);
	ScreenToClient(&currentWindowRect);

	if (_windowRect.IsRectNull() || !EqualRect(&_windowRect, &currentWindowRect))
	{
		CopyRect(&_windowRect, &currentWindowRect);
		KeyArray key_array;
		key_array.push_back(_canvas.GetWindowKey());
		KeyPath(key_array).ComputeTransform(HPS::Coordinate::Space::Pixel, HPS::Coordinate::Space::Window, _pixelToWindowMatrix);
	}

	return _pixelToWindowMatrix;
}

//! [build_mouse_event]
HPS::MouseEvent CHPSView::BuildMouseEvent(HPS::MouseEvent::Action action, HPS::MouseButtons buttons, CPoint cpoint, UINT flags, size_t click_count, float scalar)
{
	// Convert location to window space
	HPS::Point				p(static_cast<float>(cpoint.x), static_cast<float>(cpoint.y), 0);
	p = GetPixelToWindowMatrix().Transform(p);

	if(action == HPS::MouseEvent::Action::Scroll)
		return HPS::MouseEvent(action, scalar, p, MapModifierKeys(flags), click_count); 
	else
		return HPS::MouseEvent(action, p, buttons, MapModifierKeys(flags), click_count);
}
//! [build_mouse_event]

//! [left_button_down]
void CHPSView::OnLButtonDown(UINT nFlags, CPoint point)
{
	SetCapture();
	_canvas.GetWindowKey().GetEventDispatcher().InjectEvent(
		BuildMouseEvent(HPS::MouseEvent::Action::ButtonDown, HPS::MouseButtons::ButtonLeft(), point, nFlags, 1));

	CView::OnLButtonDown(nFlags, point);
}
//! [left_button_down]

void CHPSView::OnLButtonUp(UINT nFlags, CPoint point)
{
	_canvas.GetWindowKey().GetEventDispatcher().InjectEvent(
		BuildMouseEvent(HPS::MouseEvent::Action::ButtonUp, HPS::MouseButtons::ButtonLeft(), point, nFlags, 0));

	ReleaseCapture();
	SetCursor(theApp.LoadStandardCursor(IDC_ARROW));

	CView::OnLButtonUp(nFlags, point);
}

void CHPSView::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	_canvas.GetWindowKey().GetEventDispatcher().InjectEvent(
		BuildMouseEvent(HPS::MouseEvent::Action::ButtonDown, HPS::MouseButtons::ButtonLeft(), point, nFlags, 2));

	ReleaseCapture();

	CView::OnLButtonUp(nFlags, point);
}

void CHPSView::OnMouseMove(UINT nFlags, CPoint point)
{
	CRect wndRect;
	GetWindowRect(&wndRect);
	ScreenToClient(&wndRect);

	if (wndRect.PtInRect(point) || (nFlags & MK_LBUTTON) || (nFlags & MK_RBUTTON))
	{
		_canvas.GetWindowKey().GetEventDispatcher().InjectEvent(
			BuildMouseEvent(HPS::MouseEvent::Action::Move, HPS::MouseButtons(), point, nFlags, 0));
	}
	else
	{
		if(!(nFlags & MK_LBUTTON) && GetCapture() != NULL)
			OnLButtonUp(nFlags, point);
		if(!(nFlags & MK_RBUTTON) && GetCapture() != NULL)
			OnRButtonUp(nFlags, point);
	}

	CView::OnMouseMove(nFlags, point);
}

void CHPSView::OnRButtonDown(UINT nFlags, CPoint point)
{
	_canvas.GetWindowKey().GetEventDispatcher().InjectEvent(
		BuildMouseEvent(HPS::MouseEvent::Action::ButtonDown, HPS::MouseButtons::ButtonRight(), point, nFlags, 1));

	CView::OnRButtonDown(nFlags, point);
}

void CHPSView::OnRButtonUp(UINT nFlags, CPoint point)
{
	_canvas.GetWindowKey().GetEventDispatcher().InjectEvent(
		BuildMouseEvent(HPS::MouseEvent::Action::ButtonUp, HPS::MouseButtons::ButtonRight(), point, nFlags, 0));

	SetCursor(theApp.LoadStandardCursor(IDC_ARROW));

	CView::OnRButtonUp(nFlags, point);
}

void CHPSView::OnRButtonDblClk(UINT nFlags, CPoint point)
{
	_canvas.GetWindowKey().GetEventDispatcher().InjectEvent(
		BuildMouseEvent(HPS::MouseEvent::Action::ButtonDown, HPS::MouseButtons::ButtonRight(), point, nFlags, 2));

	ReleaseCapture();

	CView::OnLButtonUp(nFlags, point);
}

void CHPSView::OnMButtonDown(UINT nFlags, CPoint point)
{
	_canvas.GetWindowKey().GetEventDispatcher().InjectEvent(
		BuildMouseEvent(HPS::MouseEvent::Action::ButtonDown, HPS::MouseButtons::ButtonMiddle(), point, nFlags, 1));

	CView::OnMButtonDown(nFlags, point);
}

void CHPSView::OnMButtonUp(UINT nFlags, CPoint point)
{
	_canvas.GetWindowKey().GetEventDispatcher().InjectEvent(
		BuildMouseEvent(HPS::MouseEvent::Action::ButtonUp, HPS::MouseButtons::ButtonMiddle(), point, nFlags, 0));

	SetCursor(theApp.LoadStandardCursor(IDC_ARROW));

	CView::OnMButtonUp(nFlags, point);
}

void CHPSView::OnMButtonDblClk(UINT nFlags, CPoint point)
{
	_canvas.GetWindowKey().GetEventDispatcher().InjectEvent(
		BuildMouseEvent(HPS::MouseEvent::Action::ButtonDown, HPS::MouseButtons::ButtonMiddle(), point, nFlags, 2));

	ReleaseCapture();

	CView::OnLButtonUp(nFlags, point);
}

BOOL CHPSView::OnMouseWheel(UINT nFlags, short zDelta, CPoint point)
{
	HPS::WindowHandle hwnd;
	static_cast<ApplicationWindowKey>(_canvas.GetWindowKey()).GetWindowOptionsControl().ShowWindowHandle(hwnd);
	::ScreenToClient(reinterpret_cast<HWND>(hwnd), &point);
	_canvas.GetWindowKey().GetEventDispatcher().InjectEvent(
		BuildMouseEvent(HPS::MouseEvent::Action::Scroll, HPS::MouseButtons(), point, nFlags, 0, static_cast<float>(zDelta)));

	return CView::OnMouseWheel(nFlags, zDelta, point);
}

void CHPSView::UpdateOperatorStates(SandboxOperators op)
{
	for (auto it = _operatorStates.begin(), e = _operatorStates.end(); it != e; ++it)
		(*it).second = false;

	_operatorStates[op] = true;
}

void CHPSView::OnOperatorsOrbit()
{
	GetCanvas().GetFrontView().GetOperatorControl().Pop();
	GetCanvas().GetFrontView().GetOperatorControl().Push(new HPS::OrbitOperator(MouseButtons::ButtonLeft()));
	UpdateOperatorStates(SandboxOperators::OrbitOperator);
}


void CHPSView::OnOperatorsPan()
{
	GetCanvas().GetFrontView().GetOperatorControl().Pop();
	GetCanvas().GetFrontView().GetOperatorControl().Push(new HPS::PanOperator(MouseButtons::ButtonLeft()));
	UpdateOperatorStates(SandboxOperators::PanOperator);
}


void CHPSView::OnOperatorsZoomArea()
{
	GetCanvas().GetFrontView().GetOperatorControl().Pop();
	GetCanvas().GetFrontView().GetOperatorControl().Push(new HPS::ZoomBoxOperator(MouseButtons::ButtonLeft()));
	UpdateOperatorStates(SandboxOperators::ZoomAreaOperator);
}


void CHPSView::OnOperatorsFly()
{
	GetCanvas().GetFrontView().GetOperatorControl().Pop();
	GetCanvas().GetFrontView().GetOperatorControl().Push(new HPS::FlyOperator());
	UpdateOperatorStates(SandboxOperators::FlyOperator);
}

//! [OnOperatorsHome]
void CHPSView::OnOperatorsHome()
{
	CHPSDoc * doc = GetCHPSDoc();
	HPS::CADModel cadModel = doc->GetCADModel();
	try
	{
		if (!cadModel.Empty())
			AttachViewWithSmoothTransition(cadModel.ActivateDefaultCapture().FitWorld());
		else
			_canvas.GetFrontView().SmoothTransition(doc->GetDefaultCamera());
	}
	catch(HPS::InvalidSpecificationException const &)
	{
		// SmoothTransition() can throw if there is no model attached
	}
}
//! [OnOperatorsHome]

//! [OnOperatorsZoomFit]
void CHPSView::OnOperatorsZoomFit()
{
	HPS::View frontView = GetCanvas().GetFrontView();
	HPS::CameraKit zoomFitCamera;
	if (!_zoomToKeyPath.Empty())
	{
		HPS::BoundingKit bounding;
		_zoomToKeyPath.ShowNetBounding(true, bounding);
		frontView.ComputeFitWorldCamera(bounding, zoomFitCamera);
	}
	else
		frontView.ComputeFitWorldCamera(zoomFitCamera);
	frontView.SmoothTransition(zoomFitCamera);
}
//! [OnOperatorsZoomFit]

//! [enable_point_select_operator]
void CHPSView::OnOperatorsSelectPoint()
{
	GetCanvas().GetFrontView().GetOperatorControl().Pop();
	GetCanvas().GetFrontView().GetOperatorControl().Push(new SandboxHighlightOperator(this));
	UpdateOperatorStates(SandboxOperators::PointOperator);
}
//! [enable_point_select_operator]


void CHPSView::OnOperatorsSelectArea()
{
	GetCanvas().GetFrontView().GetOperatorControl().Pop();
	GetCanvas().GetFrontView().GetOperatorControl().Push(new HPS::HighlightAreaOperator(MouseButtons::ButtonLeft()));
	UpdateOperatorStates(SandboxOperators::AreaOperator);
}


//! [OnModesSimpleShadow]
void CHPSView::OnModesSimpleShadow()
{
	// Toggle state and bail early if we're disabling
	_enableSimpleShadows = !_enableSimpleShadows;
	if (_enableSimpleShadows == false)
	{
		GetCanvas().GetFrontView().GetSegmentKey().GetVisualEffectsControl().SetSimpleShadow(false);
		_canvas.Update();
		return;
	}

	UpdatePlanes();
}
//! [OnModesSimpleShadow]


//! [OnModesSmooth]
void CHPSView::OnModesSmooth()
{
	if (!_smoothRendering)
	{
		_canvas.GetFrontView().SetRenderingMode(Rendering::Mode::Phong);

		if (GetCHPSDoc()->GetCADModel().Type() == HPS::Type::DWGCADModel)
			_canvas.GetFrontView().GetSegmentKey().GetVisibilityControl().SetLines(true);


		_canvas.Update();
		_smoothRendering = true;
	}
}
//! [OnModesSmooth]

//! [OnModesHiddenLine]
void CHPSView::OnModesHiddenLine()
{
	if (_smoothRendering)
	{
		_canvas.GetFrontView().SetRenderingMode(Rendering::Mode::FastHiddenLine);
		_canvas.Update();
		_smoothRendering = false;

		// fixed framerate is not compatible with hidden line mode
		if (_enableFrameRate)
		{
			_canvas.SetFrameRate(0);
			_enableFrameRate = false;
		}
	}
}
//! [OnModesHiddenLine]

//! [OnModesEyeDome]
void CHPSView::OnModesEyeDome()
{
	_eyeDome = !_eyeDome;

	UpdateEyeDome(true);
}
//! [OnModesEyeDome]

//! [OnModesFrameRate]

void CHPSView::OnModesFrameRate()
{
	const float					frameRate = 20.0f;

	// Toggle frame rate and set.  Note that 0 disables frame rate.
	_enableFrameRate = !_enableFrameRate;
	if (_enableFrameRate)
		_canvas.SetFrameRate(frameRate);
	else
		_canvas.SetFrameRate(0);

	// fixed framerate is not compatible with hidden line mode
	if(!_smoothRendering)
	{
		_canvas.GetFrontView().SetRenderingMode(Rendering::Mode::Phong);
		_smoothRendering = true;
	}

	_canvas.Update();
}
//! [OnModesFrameRate]



void CHPSView::OnEditCopy()
{
	HPS::Hardcopy::GDI::ExportOptionsKit exportOptionsKit;
	HPS::Hardcopy::GDI::ExportClipboard(GetCanvas().GetWindowKey(), exportOptionsKit);
}


void CHPSView::OnFileOpen()
{
#ifndef USING_EXCHANGE || USING_EXCHANGE_PARASOLID
	CString filter = _T("HOOPS Stream Files (*.hsf)|*.hsf|StereoLithography Files (*.stl)|*.stl|WaveFront Files (*.obj)|*.obj|Point Cloud Files (*.ptx, *.pts, *.xyz)|*.ptx;*.pts;*.xyz|");
#else
	CString filter =
		_T("All CAD Files (*.3ds, *.3mf, *.3dxml, *.sat, *.sab, *_pd, *.model, *.dlv, *.exp, *.session, *.CATPart, *.CATProduct, *.CATShape, *.CATDrawing")
		_T(", *.cgr, *.dae, *.prt, *.prt.*, *.neu, *.neu.*, *.asm, *.asm.*, *.xas, *.xpr, *.fbx, *.gltf, *.glb, *.arc, *.unv, *.mf1, *.prt, *.pkg, *.ifc, *.ifczip, *.igs, *.iges, *.ipt, *.iam")
		_T(", *.jt, *.kmz, *.nwd, *.prt, *.pdf, *.prc, *.x_t, *.xmt, *.x_b, *.xmt_txt, *.rvt, *.3dm, *.stp, *.step, *.stpz, *.stp.z, *.stl, *.par, *.asm, *.pwd, *.psm")
		_T(", *.sldprt, *.sldasm, *.sldfpp, *.asm, *.u3d, *.vda, *.wrl, *.vrml, *.obj, *.xv3, *.xv0, *.hsf, *.ptx, *.pts, *.xyz)|")
		_T("*.3ds;*.3dxml;*.sat;*.sab;*_pd;*.model;*.dlv;*.exp;*.session;*.catpart;*.catproduct;*.catshape;*.catdrawing")
		_T(";*.cgr;*.dae;*.prt;*.prt.*;*.neu;*.neu.*;*.asm;*.asm.*;*.xas;*.xpr;*.fbx;*.gltf;*.glb;*.arc;*.unv;*.mf1;*.prt;*.pkg;*.ifc;*.ifczip;*.igs;*.iges;*.ipt;*.iam")
		_T(";*.jt;*.kmz;*.prt;*.pdf;*.prc;*.x_t;*.xmt;*.x_b;*.xmt_txt;*.rvt;*.3dm;*.stp;*.step;*.stpz;*.stp.z;*.stl;*.par;*.asm;*.pwd;*.psm")
		_T(";*.sldprt;*.sldasm;*.sldfpp;*.asm;*.u3d;*.vda;*.wrl;*.vrml;*.obj;*.xv3;*.xv0;*.hsf;*.ptx;*.pts;*.xyz|")
		_T("3D Studio Files (*.3ds)|*.3ds|")
		_T("3D Manufacturing Files (*.3mf)|*.3mf|")
		_T("3DXML Files (*.3dxml)|*.3dxml|")
		_T("ACIS SAT Files (*.sat, *.sab)|*.sat;*.sab|")
		_T("CADDS Files (*_pd)|*_pd|")
		_T("CATIA V4 Files (*.model, *.dlv, *.exp, *.session)|*.model;*.dlv;*.exp;*.session|")
		_T("CATIA V5 Files (*.CATPart, *.CATProduct, *.CATShape, *.CATDrawing)|*.catpart;*.catproduct;*.catshape;*.catdrawing|")
		_T("CGR Files (*.cgr)|*.cgr|")
		_T("Collada Files (*.dae)|*.dae|")
		_T("Creo (ProE) Files (*.prt, *.prt.*, *.neu, *.neu.*, *.asm, *.asm.*, *.xas, *.xpr)|*.prt;*.prt.*;*.neu;*.neu.*;*.asm;*.asm.*;*.xas;*.xpr|")
		_T("FBX Files (*.fbx)|*.fbx|")
		_T("GLTF Files (*.gltf, *.glb)|*.gltf;*.glb|")
		_T("I-DEAS Files (*.arc, *.unv, *.mf1, *.prt, *.pkg)|*.arc;*.unv;*.mf1;*.prt;*.pkg|")
		_T("IFC Files (*.ifc, *.ifczip)|*.ifc;*.ifczip|")
		_T("IGES Files (*.igs, *.iges)|*.igs;*.iges|")
		_T("Inventor Files (*.ipt, *.iam)|*.ipt;*.iam|")
		_T("JT Files (*.jt)|*.jt|")
		_T("KMZ Files (*.kmz)|*.kmz|")
		_T("Navisworks Files (*.nwd)|*.nwd|")
		_T("NX (Unigraphics) Files (*.prt)|*.prt|")
		_T("PDF Files (*.pdf)|*.pdf|")
		_T("PRC Files (*.prc)|*.prc|")
		_T("Parasolid Files (*.x_t, *.xmt, *.x_b, *.xmt_txt)|*.x_t;*.xmt;*.x_b;*.xmt_txt|")
		_T("Revit Files (*.rvt)|*.rvt|")
		_T("Rhino Files (*.3dm)|*.3dm|")
		_T("SolidEdge Files (*.par, *.asm, *.pwd, *.psm)|*.par;*.asm;*.pwd;*.psm|")
		_T("SolidWorks Files (*.sldprt, *.sldasm, *.sldfpp, *.asm)|*.sldprt;*.sldasm;*.sldfpp;*.asm|")
		_T("STEP Files (*.stp, *.step, *.stpz, *.stp.z)|*.stp;*.step;*.stpz;*.stp.z|")
		_T("STL Files (*.stl)|*.stl|")
		_T("Universal 3D Files (*.u3d)|*.u3d|")
		_T("VDA Files (*.vda)|*.vda|")
		_T("VRML Files (*.wrl, *.vrml)|*.wrl;*.vrml|")
		_T("XVL Files (*.xv3, *.xv0)|*.xv0;*.xv3|")
		_T("HOOPS Stream Files (*.hsf)|*.hsf|StereoLithography Files (*.stl)|*.stl|WaveFront Files (*.obj)|*.obj|Point Cloud Files (*.ptx, *.pts, *.xyz)|*.ptx;*.pts;*.xyz|");
#endif

#if defined(USING_PARASOLID)
	CString parasolid_filter = _T("Parasolid Files (*.x_t, *.x_b, *.xmt_txt, *.xmt_bin)|*.x_t;*.x_b;*.xmt_txt;*.xmt_bin|");
	filter.Append(parasolid_filter);
#endif

#if defined (USING_DWG)
	CString dwg_filter = _T("DWG Files (*.dwg, *.dxf)|*.dwg;*.dxf|");
	filter.Append(dwg_filter);
#endif

	CString all_files = _T("All Files (*.*)|*.*||");
	filter.Append(all_files);
	CFileDialog dlg(TRUE, _T(".hsf"), NULL, OFN_HIDEREADONLY, filter, NULL);


	if (dlg.DoModal() == IDOK)
	{
		auto t0 = std::chrono::system_clock::now();
		GetDocument()->OnOpenDocument(dlg.GetPathName());

		CADModel cadModel = GetDocument()->GetCADModel();
		if (Type::ExchangeCADModel != cadModel.Type())
			return;
		
		auto t1 = std::chrono::system_clock::now();

		auto dur1 = t1 - t0;
		auto msec1 = std::chrono::duration_cast<std::chrono::milliseconds>(dur1).count();

		wchar_t wcsbuf[512];
		swprintf(wcsbuf, sizeof(wcsbuf) / sizeof(wchar_t), L"Loading time: %d msec", (int)msec1);

		ShowMessage(wcsbuf);

#ifdef USING_EXCHANGE_PARASOLID

		((ExPsProcess*)m_pProcess)->SetModelFile(((HPS::Exchange::CADModel)cadModel).GetExchangeEntity());
		if (((ExPsProcess*)m_pProcess)->m_bIsPart)
		{
			// Create component path for adding body
			HPS::ComponentArray compPathArr;

			HPS::Component comp = cadModel;;
			compPathArr.push_back(comp);

			HPS::Component::ComponentType compType = comp.GetComponentType();
			while (HPS::Component::ComponentType::ExchangePartDefinition != compType)
			{
				HPS::ComponentArray compArr = comp.GetSubcomponents();
				comp = compArr[0];
				compType = comp.GetComponentType();

				compPathArr.insert(compPathArr.begin(), comp);
			}
			m_addCompPath = HPS::ComponentPath(compPathArr);
		}
		else
		{
			m_addCompPath = HPS::ComponentPath();
		}

		// Create PsComponent mapper
		if (NULL != m_psMapper)
			delete m_psMapper;
		m_psMapper = new PsComponentMapper(cadModel);

#else
		((ExProcess*)m_pProcess)->SetModelFile(((HPS::Exchange::CADModel)cadModel).GetExchangeEntity());
#endif


		// AxisTriad and NaviCube
		View view = GetCanvas().GetFrontView();
		view.GetAxisTriadControl().SetVisibility(true).SetInteractivity(true);
		view.GetNavigationCubeControl().SetVisibility(true).SetInteractivity(true);
		GetCanvas().Update();
	}
}

void CHPSView::OnFileSaveAs()
{
	CString filter = _T("HOOPS Stream Files (*.hsf)|*.hsf|PDF (*.pdf)|*.pdf|Postscript Files (*.ps)|*.ps|JPEG Image File(*.jpeg)|*.jpeg|PNG Image Files (*.png)|*.png|");
#ifdef USING_EXCHANGE
	CString exchange_filter =
		_T("PRC Files (*.prc)|*.prc|");

	exchange_filter.Append(filter);
	filter = exchange_filter;
#endif

	CFileDialog dlg(FALSE, _T(".hsf"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_NOREADONLYRETURN, filter, NULL);

	if (dlg.DoModal())
	{
		CString pathname;
		pathname = dlg.GetPathName();
		CStringA ansiPath(pathname);

		char ext[5];
		ext[0]= '\0';

		char const * tmp = ::strrchr(ansiPath, '.');
		if(!tmp)
		{
			strcpy_s(ext, ansiPath);
			return;
		}

		++tmp;
		size_t len = strlen(tmp);
		for (size_t i = 0; i <= len; ++i){
			ext[i] = static_cast<char>(tolower(tmp[i]));
		}

		if (strcmp(ext, "hsf") == 0)
		{
			//! [export_hsf]
			HPS::Stream::ExportOptionsKit eok;
			HPS::SegmentKey exportFromHere;

			HPS::Model model = _canvas.GetFrontView().GetAttachedModel();
			if (model.Type() == HPS::Type::None)
				exportFromHere = _canvas.GetFrontView().GetSegmentKey();
			else
				exportFromHere = model.GetSegmentKey();

			HPS::Stream::ExportNotifier notifier;
			HPS::IOResult status = IOResult::Failure;
			try
			{
				notifier = HPS::Stream::File::Export(ansiPath, exportFromHere, eok); 
				CProgressDialog edlg(GetDocument(), notifier, true);
				edlg.DoModal();
				status = notifier.Status();
			}
			//! [export_hsf]
			catch (HPS::IOException const & e)
			{
				char what[1024];
				sprintf_s(what, 1024, "HPS::Stream::File::Export threw an exception: %s", e.what());
				MessageBoxA(NULL, what, NULL, 0);
			}
			if (status != HPS::IOResult::Success && status != HPS::IOResult::Canceled)
				GetParentFrame()->MessageBox(L"HPS::Stream::File::Export encountered an error", _T("File export error"), MB_ICONERROR | MB_OK);

		}
		else if (strcmp(ext, "pdf") == 0)
		{
			bool export_2d_pdf = true;
#ifdef USING_PUBLISH
			PDFExportDialog pdf_export;
			if (pdf_export.DoModal() == IDOK)
				export_2d_pdf = pdf_export.Export2DPDF();
			else
				return;

			if (!export_2d_pdf)
			{
				//! [export_3d_pdf]
				try 
				{
					HPS::Publish::ExportOptionsKit export_kit;
					CADModel cad_model = GetDocument()->GetCADModel();
					if (cad_model.Type() == HPS::Type::ExchangeCADModel)
						HPS::Publish::File::ExportPDF(cad_model, ansiPath, export_kit);
					else
					{
						HPS::SprocketPath sprocket_path(GetCanvas(), GetCanvas().GetAttachedLayout(), GetCanvas().GetFrontView(), GetCanvas().GetFrontView().GetAttachedModel());
						HPS::Publish::File::ExportPDF(sprocket_path.GetKeyPath(), ansiPath, export_kit);
					}
				}
				catch (HPS::IOException const & e)
				{
					char what[1024];
					sprintf_s(what, 1024, "HPS::Publish::File::Export threw an exception: %s", e.what());
					MessageBoxA(NULL, what, NULL, 0);
				}
				//! [export_3d_pdf]
			}
#endif
			if (export_2d_pdf)
			{
				//! [export_2d_pdf]
				try 
				{
					HPS::Hardcopy::File::Export(ansiPath, HPS::Hardcopy::File::Driver::PDF,
								_canvas.GetWindowKey(), HPS::Hardcopy::File::ExportOptionsKit::GetDefault());
				}
				catch (HPS::IOException const & e)
				{
					char what[1024];
					sprintf_s(what, 1024, "HPS::Hardcopy::File::Export threw an exception: %s", e.what());
					MessageBoxA(NULL, what, NULL, 0);
				}
				//! [export_2d_pdf]
			}
		}
		else if (strcmp(ext, "ps") == 0)
		{
			try 
			{
				HPS::Hardcopy::File::Export(ansiPath, HPS::Hardcopy::File::Driver::Postscript,
						_canvas.GetWindowKey(), HPS::Hardcopy::File::ExportOptionsKit::GetDefault());
			}
			catch (HPS::IOException const & e)
			{
				char what[1024];
				sprintf_s(what, 1024, "HPS::Hardcopy::File::Export threw an exception: %s", e.what());
				MessageBoxA(NULL, what, NULL, 0);
			}
		}
		else if (strcmp(ext, "jpeg") == 0)
		{
			HPS::Image::ExportOptionsKit eok;
			eok.SetFormat(Image::Format::Jpeg);

			try { HPS::Image::File::Export(ansiPath, _canvas.GetWindowKey(), eok); }
			catch (HPS::IOException const & e)
			{
				char what[1024];
				sprintf_s(what, 1024, "HPS::Image::File::Export threw an exception: %s", e.what());
				MessageBoxA(NULL, what, NULL, 0);
			}
		}
		else if (strcmp(ext, "png") == 0)
		{
			try { HPS::Image::File::Export(ansiPath, _canvas.GetWindowKey(), HPS::Image::ExportOptionsKit::GetDefault()); }
			catch (HPS::IOException const & e)
			{
				char what[1024];
				sprintf_s(what, 1024, "HPS::Image::File::Export threw an exception: %s", e.what());
				MessageBoxA(NULL, what, NULL, 0);
			}
		}
#ifdef USING_EXCHANGE
		else if (strcmp(ext, "prc") == 0)
		{
			HPS::Exchange::ExportPRCOptionsKit export_kit = HPS::Exchange::ExportPRCOptionsKit::GetDefault();
			CADModel cad_model = GetDocument()->GetCADModel();
			if (HPS::Type::ExchangeCADModel == cad_model.Type())
			{
#ifdef USING_EXCHANGE_PARASOLID
				A3DAsmModelFile* pCopyModelFile;
				((ExPsProcess*)m_pProcess)->CopyAndUpdateModelFile(pCopyModelFile);

				HPS::Exchange::CADModel copy_cad_model = HPS::Exchange::Factory::CreateCADModel(HPS::Factory::CreateModel(), pCopyModelFile);

				HPS::Exchange::File::ExportPRC(copy_cad_model, ansiPath, export_kit);
#else
				HPS::Exchange::File::ExportPRC(cad_model, ansiPath, export_kit);
#endif
			}
			else
			{
				HPS::SprocketPath sprocket_path(GetCanvas(), GetCanvas().GetAttachedLayout(), GetCanvas().GetFrontView(), GetCanvas().GetFrontView().GetAttachedModel());
				HPS::Exchange::File::ExportPRC(sprocket_path.GetKeyPath(), ansiPath);
			}
		}
#endif
	}
}

void CHPSView::UpdateRenderingMode(HPS::Rendering::Mode mode)
{
	if (mode == HPS::Rendering::Mode::Phong)
		_smoothRendering = true;
	else if (mode == HPS::Rendering::Mode::FastHiddenLine)
		_smoothRendering = false;
}

void CHPSView::UpdatePlanes()
{
	if(_enableSimpleShadows)
	{
		GetCanvas().GetFrontView().SetSimpleShadow(true);
	
		const float					opacity = 0.3f;
		const unsigned int			resolution = 512;
		const unsigned int			blurring = 20;

		HPS::SegmentKey viewSegment = GetCanvas().GetFrontView().GetSegmentKey();

		// Set opacity in simple shadow color
		HPS::RGBAColor color(0.4f, 0.4f, 0.4f, opacity);
		if (viewSegment.GetVisualEffectsControl().ShowSimpleShadowColor(color))
			color.alpha = opacity;

		viewSegment.GetVisualEffectsControl()
			.SetSimpleShadow(_enableSimpleShadows, resolution, blurring)
			.SetSimpleShadowColor(color);
		_canvas.Update();
	}
}

void CHPSView::OnUpdateRibbonBtnSimpleShadow(CCmdUI * pCmdUI)
{
	pCmdUI->SetCheck(_enableSimpleShadows);
}

void CHPSView::OnUpdateRibbonBtnFrameRate(CCmdUI * pCmdUI)
{
	pCmdUI->SetCheck(_enableFrameRate);
}

void CHPSView::OnUpdateRibbonBtnSmooth(CCmdUI * pCmdUI)
{
	pCmdUI->SetCheck(_smoothRendering);
}

void CHPSView::OnUpdateRibbonBtnHiddenLine(CCmdUI * pCmdUI)
{
	pCmdUI->SetCheck(!_smoothRendering);
}

void CHPSView::OnUpdateRibbonBtnEyeDome(CCmdUI * pCmdUI)
{
	pCmdUI->SetCheck(_eyeDome);
}

void CHPSView::OnUpdateRibbonBtnOrbitOp(CCmdUI * pCmdUI)
{
	pCmdUI->SetCheck(_operatorStates[SandboxOperators::OrbitOperator]);
}

void CHPSView::OnUpdateRibbonBtnPanOp(CCmdUI * pCmdUI)
{
	pCmdUI->SetCheck(_operatorStates[SandboxOperators::PanOperator]);
}

void CHPSView::OnUpdateRibbonBtnZoomAreaOp(CCmdUI * pCmdUI)
{
	pCmdUI->SetCheck(_operatorStates[SandboxOperators::ZoomAreaOperator]);
}

void CHPSView::OnUpdateRibbonBtnFlyOp(CCmdUI * pCmdUI)
{
	pCmdUI->SetCheck(_operatorStates[SandboxOperators::FlyOperator]);
}

void CHPSView::OnUpdateRibbonBtnPointOp(CCmdUI * pCmdUI)
{
	pCmdUI->SetCheck(_operatorStates[SandboxOperators::PointOperator]);
}

void CHPSView::OnUpdateRibbonBtnAreaOp(CCmdUI * pCmdUI)
{
	pCmdUI->SetCheck(_operatorStates[SandboxOperators::AreaOperator]);
}

void CHPSView::OnKillFocus(CWnd* /*pNewWnd*/)
{
	_canvas.GetWindowKey().GetEventDispatcher().InjectEvent(HPS::FocusLostEvent());
}

void CHPSView::OnSetFocus(CWnd* /*pOldWnd*/)
{
	_capsLockState = IsCapsLockOn();
}

void CHPSView::initOperators()
{
	GetCanvas().GetFrontView().GetOperatorControl().UnsetEverything();
	SetupDefaultOperators();

	GetCanvas().Update();
}

//! [user_code]
void CHPSView::OnUserCode1()
{
	// Toggle display of resource monitor using the DebuggingControl
	_displayResourceMonitor = !_displayResourceMonitor;
	_canvas.GetWindowKey().GetDebuggingControl().SetResourceMonitor(_displayResourceMonitor);

	/*
	HPS::CADModel cadModel = GetDocument()->GetCADModel();
	if (HPS::Type::None == cadModel.Type())
		return;

	CString filter =
		_T("All CAD Files (*.3ds, *.3dxml, *.sat, *.sab, *_pd, *.model, *.dlv, *.exp, *.session, *.CATPart, *.CATProduct, *.CATShape, *.CATDrawing")
		_T(", *.cgr, *.dae, *.prt, *.prt.*, *.neu, *.neu.*, *.asm, *.asm.*, *.xas, *.xpr, *.arc, *.unv, *.mf1, *.prt, *.pkg, *.ifc, *.ifczip, *.igs, *.iges, *.ipt, *.iam")
		_T(", *.jt, *.kmz, *.prt, *.pdf, *.prc, *.x_t, *.xmt, *.x_b, *.xmt_txt, *.3dm, *.stp, *.step, *.stpz, *.stp.z, *.stl, *.par, *.asm, *.pwd, *.psm")
		_T(", *.sldprt, *.sldasm, *.sldfpp, *.asm, *.u3d, *.vda, *.wrl, *.vml, *.obj, *.xv3, *.xv0)|")
		_T("*.3ds;*.3dxml;*.sat;*.sab;*_pd;*.model;*.dlv;*.exp;*.session;*.catpart;*.catproduct;*.catshape;*.catdrawing")
		_T(";*.cgr;*.dae;*.prt;*.prt.*;*.neu;*.neu.*;*.asm;*.asm.*;*.xas;*.xpr;*.arc;*.unv;*.mf1;*.prt;*.pkg;*.ifc;*.ifczip;*.igs;*.iges;*.ipt;*.iam")
		_T(";*.jt;*.kmz;*.prt;*.pdf;*.prc;*.x_t;*.xmt;*.x_b;*.xmt_txt;*.3dm;*.stp;*.step;*.stpz;*.stp.z;*.stl;*.par;*.asm;*.pwd;*.psm")
		_T(";*.sldprt;*.sldasm;*.sldfpp;*.asm;*.u3d;*.vda;*.wrl;*.vml;*.obj;*.xv3;*.xv0;*.hsf|")
		_T("3D Studio Files (*.3ds)|*.3ds|")
		_T("3DXML Files (*.3dxml)|*.3dxml|")
		_T("ACIS SAT Files (*.sat, *.sab)|*.sat;*.sab|")
		_T("CADDS Files (*_pd)|*_pd|")
		_T("CATIA V4 Files (*.model, *.dlv, *.exp, *.session)|*.model;*.dlv;*.exp;*.session|")
		_T("CATIA V5 Files (*.CATPart, *.CATProduct, *.CATShape, *.CATDrawing)|*.catpart;*.catproduct;*.catshape;*.catdrawing|")
		_T("CGR Files (*.cgr)|*.cgr|")
		_T("Collada Files (*.dae)|*.dae|")
		_T("Creo (ProE) Files (*.prt, *.prt.*, *.neu, *.neu.*, *.asm, *.asm.*, *.xas, *.xpr)|*.prt;*.prt.*;*.neu;*.neu.*;*.asm;*.asm.*;*.xas;*.xpr|")
		_T("I-DEAS Files (*.arc, *.unv, *.mf1, *.prt, *.pkg)|*.arc;*.unv;*.mf1;*.prt;*.pkg|")
		_T("IFC Files (*.ifc, *.ifczip)|*.ifc;*.ifczip|")
		_T("IGES Files (*.igs, *.iges)|*.igs;*.iges|")
		_T("Inventor Files (*.ipt, *.iam)|*.ipt;*.iam|")
		_T("JT Files (*.jt)|*.jt|")
		_T("KMZ Files (*.kmz)|*.kmz|")
		_T("NX (Unigraphics) Files (*.prt)|*.prt|")
		_T("PDF Files (*.pdf)|*.pdf|")
		_T("PRC Files (*.prc)|*.prc|")
		_T("Parasolid Files (*.x_t, *.xmt, *.x_b, *.xmt_txt)|*.x_t;*.xmt;*.x_b;*.xmt_txt|")
		_T("Rhino Files (*.3dm)|*.3dm|")
		_T("STEP Files (*.stp, *.step, *.stpz, *.stp.z)|*.stp;*.step;*.stpz;*.stp.z|")
		_T("STL Files (*.stl)|*.stl|")
		_T("SolidEdge Files (*.par, *.asm, *.pwd, *.psm)|*.par;*.asm;*.pwd;*.psm|")
		_T("SolidWorks Files (*.sldprt, *.sldasm, *.sldfpp, *.asm)|*.sldprt;*.sldasm;*.sldfpp;*.asm|")
		_T("Universal 3D Files (*.u3d)|*.u3d|")
		_T("VDA Files (*.vda)|*.vda|")
		_T("VRML Files (*.wrl, *.vrml)|*.wrl;*.vrml|")
		_T("XVL Files (*.xv3, *.xv0)|*.xv0;*.xv3|");

	CFileDialog dlg(TRUE, _T(".*"), NULL, OFN_HIDEREADONLY, filter, NULL);
	if (dlg.DoModal() != IDOK)
		return;

	CString lpszPathName = dlg.GetPathName();
	HPS::UTF8 filename(lpszPathName);

	HPS::IOResult status = HPS::IOResult::Failure;
	HPS::Exchange::ImportNotifier notifier;

	try
	{
		HPS::Exchange::ImportOptionsKit importOptKit = HPS::Exchange::ImportOptionsKit::GetDefault();

		//HPS::ComponentPath topLevelPath(HPS::ComponentArray(1, GetDocument()->GetCADModel()));
		
		HPS::Exchange::CADModel exCadModel = cadModel;
		HPS::ComponentArray compArr = exCadModel.GetSubcomponents();
		HPS::Component parentComp = compArr[0];
		HPS::UTF8 name = parentComp.GetName();

		HPS::ComponentArray compPathArr;
		compPathArr.push_back(parentComp);
		compPathArr.push_back(exCadModel);

		importOptKit.SetLocation(compPathArr);

		notifier = HPS::Exchange::File::Import(filename, importOptKit);

		//CProgressDialogAdd* addDlg = new CProgressDialogAdd();
		//addDlg->Create(IDD_PROGRESS_ADD);
		//addDlg->ShowWindow(SW_SHOW);

		notifier.Wait();

		//addDlg->DestroyWindow();

		status = notifier.Status();

		compArr = parentComp.GetSubcomponents();

		HPS::Component lastComp = compArr[compArr.size() - 1];

		HPS::UTF8 name2 = lastComp.GetName();

	}
	catch (HPS::IOException const& ex)
	{
		status = ex.result;
	}
	*/
	GetCanvas().Update();
}


HPS::Component CHPSView::GetOwnerBrepModel(HPS::Component in_comp)
{
	HPS::Component::ComponentType compType = in_comp.GetComponentType();

	// Get owner BrepModel
	HPS::ComponentArray compArr = in_comp.GetOwners();
	compType = compArr[0].GetComponentType();

	while (HPS::Component::ComponentType::ExchangeRIBRepModel != compType)
	{
		compArr = compArr[0].GetOwners();
		compType = compArr[0].GetComponentType();
	}

	HPS::Component brepComp = compArr[0];

	return brepComp;
}


HPS::SegmentKey CHPSView::prepareInfo(float& posX, float& posY)
{
	// Create info SK
	HPS::SegmentKey modelSK = GetCanvas().GetFrontView().GetAttachedModel().GetSegmentKey();
	HPS::SegmentKey infoSK = modelSK.Down("info");

	if (HPS::Type::None == infoSK.Type())
	{
		infoSK = modelSK.Subsegment("info");

		// fix camera
		HPS::CameraKit camera;
		camera.SetPosition(HPS::Point(0, 0, 1));
		camera.SetTarget(HPS::Point(0, 0, 0));
		camera.SetUpVector(HPS::Vector(0, 1, 0));
		camera.SetProjection(HPS::Camera::Projection::Stretched);
		camera.SetField(2, 2);
		infoSK.SetCamera(camera);

		// overlay
		infoSK.GetDrawingAttributeControl().SetOverlay(HPS::Drawing::Overlay::Default);

		infoSK.GetVisibilityControl().SetCuttingSections(false);
	}
	else
	{
		infoSK.Flush(Search::Type::Geometry);
	}

	// Get keypath
	HPS::Canvas canvas = GetCanvas();
	HPS::Layout layout = canvas.GetAttachedLayout();
	HPS::View view = canvas.GetFrontView();
	HPS::Model model = view.GetAttachedModel();

	HPS::SprocketPath sprkPath(canvas, layout, view, model);

	HPS::KeyPath keyPath = sprkPath.GetKeyPath();

	HPS::Point pixel;
	keyPath.ConvertCoordinate(HPS::Coordinate::Space::Window, HPS::Point(1, -1, 0), HPS::Coordinate::Space::Pixel, pixel);

	posX = -1 + 2 / pixel.x * 20;
	posY = 1 - 2 / pixel.y * 20;

	return infoSK;
}

void CHPSView::ShowMessage(wchar_t* wmag)
{
	float posX, posY;
	HPS::SegmentKey infoSK = prepareInfo(posX, posY);

	HPS::TextKit textKit;
	textKit.SetPosition(HPS::Point(posX, posY, 0));
	textKit.SetFont("Meiryo UI");
	textKit.SetColor(HPS::RGBAColor(1, 1, 1, 1));
	textKit.SetSize(18, HPS::Text::SizeUnits::Pixels);
	textKit.SetAlignment(HPS::Text::Alignment::TopLeft);
	textKit.SetBackground(true, "box");
	textKit.SetText((HPS::UTF8)wmag);
	infoSK.InsertText(textKit);

	Update();
}

void CHPSView::setCameraIso()
{
	HPS::CameraControl camera = GetCanvas().GetFrontView().GetSegmentKey().GetCameraControl();
	camera.SetPosition(HPS::Point(2.886757, 2.886757, 2.886757));
	camera.SetTarget(HPS::Point(0, 0, 0));
	camera.SetUpVector(HPS::Vector(-0.408248, -0.408248, 0.816496));

	GetCanvas().GetFrontView().FitWorld();
	GetCanvas().Update();
}

#ifdef USING_EXCHANGE_PARASOLID
void CHPSView::AddBody(const int body)
{
	HPS::CADModel cadModel = GetDocument()->GetCADModel();
	HPS::Type type = cadModel.Type();

	HPS::Exchange::CADModel exCadModel;
	HPS::Component comp;

	HPS::MatrixKit translation;
	translation.Translate(0.0f, 0.0f, 0.0f);

	if (HPS::Type::ExchangeCADModel != type)
	{
		exCadModel = HPS::Exchange::Factory::CreateCADModel();
		comp = HPS::ExchangeParasolid::File::AddEntity(exCadModel, body, translation);

		// Create PsComponent mapper
		if (NULL != m_psMapper)
			delete m_psMapper;
		m_psMapper = new PsComponentMapper(exCadModel);
	}
	else
	{
		exCadModel = (HPS::Exchange::CADModel)cadModel;
		if (((ExPsProcess*)m_pProcess)->m_bIsPart)
			//comp = HPS::ExchangeParasolid::File::AddEntity(m_addCompPath, body, translation);
			comp = HPS::ExchangeParasolid::File::AddEntity(exCadModel, body, translation);
		else
			comp = HPS::ExchangeParasolid::File::AddEntity(exCadModel, body, translation);

		// PsComponent mapper
		InitPsBodyMap((int)body, comp);
	}

	if (HPS::Type::ExchangeCADModel != type)
	{
		// Create component path for adding body
		HPS::Component::ComponentType compType = comp.GetComponentType();

		while (HPS::Component::ComponentType::ExchangePartDefinition != compType)
		{
			HPS::ComponentArray compArr = comp.GetOwners();
			comp = compArr[0];
			compType = comp.GetComponentType();
		}

		HPS::ComponentArray compPathArr;
		compPathArr.push_back(comp);

		while (HPS::Component::ComponentType::ExchangeModelFile != compType)
		{
			HPS::ComponentArray compArr = comp.GetOwners();
			comp = compArr[0];
			compType = comp.GetComponentType();

			compPathArr.push_back(comp);
		}

		m_addCompPath = HPS::ComponentPath(compPathArr);

		// Set model file
		((ExPsProcess*)m_pProcess)->SetModelFile(exCadModel.GetExchangeEntity());

		// Set model to view
		GetCanvas().GetFrontView().AttachModel(exCadModel.GetModel());

		// Set CAD model
		GetDocument()->SetCADModel(exCadModel.GetModel());

		setCameraIso();
	}
}

HPS::Component CHPSView::GetOwnerPSBodyCompo(HPS::Component in_comp)
{
	// Get owner BrepModel
	HPS::ComponentArray compArr = in_comp.GetOwners();
	HPS::Component::ComponentType compType = compArr[0].GetComponentType();

	while (HPS::Component::ComponentType::ParasolidTopoBody != compType)
	{
		compArr = compArr[0].GetOwners();
		compType = compArr[0].GetComponentType();
	}

	HPS::Component brepComp = compArr[0];

	return brepComp;
}
#endif


void CHPSView::OnUserCode2()
{
	initOperators();

	CreateSolidDlg* pDlg = new CreateSolidDlg(this);
	if (pDlg->DoModal() == IDOK)
	{
		int iShape = pDlg->m_iShape;

		double size[4] =
		{
			pDlg->m_dEditS1,
			pDlg->m_dEditS2,
			pDlg->m_dEditS3,
			pDlg->m_dHookAng
		};

		double offset[3] =
		{
			pDlg->m_dEditOX,
			pDlg->m_dEditOY,
			pDlg->m_dEditOZ
		};

		double dir[3] =
		{
			pDlg->m_dEditDX,
			pDlg->m_dEditDY,
			pDlg->m_dEditDZ
		};

#ifdef USING_EXCHANGE_PARASOLID
		PK_BODY_t body = PK_ENTITY_null;
		if (!((ExPsProcess*)m_pProcess)->CreateSolid((SolidShape)iShape, size, offset, dir, body))
			return;

		AddBody(body);

#else
		A3DAsmModelFile* pModelFile = NULL;
		if (!((ExProcess*)m_pProcess)->CreateSolid((SolidShape)iShape, size, offset, dir, pModelFile))
			return;

		if (NULL != pModelFile)
		{
			// Exchange to HPS
			HPS::Exchange::CADModel exCadModel = HPS::Exchange::Factory::CreateCADModel(HPS::Factory::CreateModel(), pModelFile);

			GetDocument()->SetCADModel((HPS::CADModel)exCadModel);

			exCadModel.Reload().Wait();

			GetCanvas().GetFrontView().AttachModel(exCadModel.GetModel());

			setCameraIso();
		}
		else
		{
			CADModel cad_model = GetDocument()->GetCADModel();
			HPS::Exchange::CADModel exCadModel = (HPS::Exchange::CADModel)cad_model;

			HPS::Exchange::ReloadNotifier notifier = exCadModel.Reload();
			notifier.Wait();

		}
#endif
		GetCanvas().Update();
	}
}


void CHPSView::OnUserCode3()
{
	initOperators();

	BlendDlg* pDlg = new BlendDlg(this, m_pProcess, this);
	pDlg->ShowWindow(SW_SHOW);

	GetCanvas().Update();
}

void CHPSView::OnUserCode4()
{
	initOperators();

	DeleteCompDlg* pDlg = new DeleteCompDlg(this, m_pProcess, ClickCompType::CLICK_PART, this);
	pDlg->ShowWindow(SW_SHOW);

}

void CHPSView::OnButtonDeleteBody()
{
	initOperators();

	DeleteCompDlg* pDlg = new DeleteCompDlg(this, m_pProcess, ClickCompType::CLICK_BODY, this);
	pDlg->ShowWindow(SW_SHOW);
}

void CHPSView::OnButtonBool()
{
	initOperators();

	BooleanDlg* pDlg = new BooleanDlg(this, m_pProcess, this);
	pDlg->ShowWindow(SW_SHOW);
}


void CHPSView::OnButtonFr()
{
	initOperators();

	FeatureRecognitionDlg* pDlg = new FeatureRecognitionDlg(this, m_pProcess, this);
	pDlg->ShowWindow(SW_SHOW);
}


void CHPSView::OnButtonDeleteFace()
{
	initOperators();

	DeleteFaceDlg* pDlg = new DeleteFaceDlg(this, m_pProcess, this);
	pDlg->ShowWindow(SW_SHOW);
}

void CHPSView::OnButtonHollow()
{
	initOperators();

	HollowDlg* pDlg = new HollowDlg(this, m_pProcess, this);
	pDlg->ShowWindow(SW_SHOW);
}





void CHPSView::OnButtonMirror()
{
	initOperators();

	MirrorDlg* pDlg = new MirrorDlg(this, m_pProcess, this);
	pDlg->ShowWindow(SW_SHOW);
}
