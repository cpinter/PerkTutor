
// VTK includes
#include <vtkCleanPolyData.h>
#include <vtkCommand.h>
#include <vtkExtractPolyDataGeometry.h>
#include <vtkExtractSelectedPolyDataIds.h>
#include <vtkIdTypeArray.h>
#include <vtkInformation.h>
#include <vtkPlanes.h>
#include <vtkSelection.h>
#include <vtkSelectionNode.h>
#include "vtkIntArray.h"
#include "vtkMath.h"
#include "vtkMatrix4x4.h"
#include "vtkSmartPointer.h"
#include "vtkObjectFactory.h"
#include "vtkImageData.h"
#include "vtkXMLDataParser.h"

// WorkflowSegmentation MRML includes
#include "vtkMRMLWorkflowSegmentationNode.h"

// MRML includes
#include <vtkMRMLDiffusionTensorDisplayPropertiesNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLAnnotationNode.h>
#include <vtkMRMLAnnotationROINode.h>
#include "vtkMRMLLinearTransformNode.h"
// STD includes
#include <math.h>
#include <vector>
#include <algorithm>
#include <cmath>
#include <ctime>
#include <fstream>
#include <sstream>
#include <string>


// MACROS ---------------------------------------------------------------------

#define DELETE_IF_NOT_NULL(x) \
  if ( x != NULL ) \
    { \
    x->Delete(); \
    x = NULL; \
    }

#define WRITE_STRING_XML(x) \
  if ( this->x != NULL ) \
  { \
    of << indent << " "#x"=\"" << this->x << "\"\n"; \
  }

#define READ_AND_SET_STRING_XML(x) \
    if ( !strcmp( attName, #x ) ) \
      { \
      this->SetAndObserve##x( NULL ); \
      this->Set##x( attValue ); \
      }


// For testing.
void fileOutput( std::string str, double num )
{
  std::ofstream output( "_vtkMRMLWorkflowSegmentationNode.txt", std::ios_base::app );
  time_t seconds;
  seconds = time( NULL );
  
  output << seconds << " : " << str << " - " << num << std::endl;
  output.close();
}



// Helper functions.
// ----------------------------------------------------------------------------

bool
StringToBool( std::string str )
{
  bool var;
  std::stringstream ss( str );
  ss >> var;
  return var;
}

// ----------------------------------------------------------------------------

vtkMRMLWorkflowSegmentationNode* vtkMRMLWorkflowSegmentationNode
::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance( "vtkMRMLWorkflowSegmentationNode" );
  if( ret )
    {
      return ( vtkMRMLWorkflowSegmentationNode* )ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLWorkflowSegmentationNode;
}



vtkMRMLNode* vtkMRMLWorkflowSegmentationNode
::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance( "vtkMRMLWorkflowSegmentationNode" );
  if( ret )
    {
      return ( vtkMRMLWorkflowSegmentationNode* )ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLWorkflowSegmentationNode;
}


void
vtkMRMLWorkflowSegmentationNode
::StartReceiveServer()
{
  this->Active = true;
}



void
vtkMRMLWorkflowSegmentationNode
::StopReceiveServer()
{
  this->Active = false;
}

void
vtkMRMLWorkflowSegmentationNode
::SaveTrackingLog( std::string fileName )
{
  std::ofstream output( fileName.c_str() );
  
  if ( ! output.is_open() )
    {
    vtkErrorMacro( "Record file could not be opened!" );
    return;
    }
  
  output << "<WorkflowSegmentationLog>" << std::endl;
  
    // Save transforms.
  
  for ( unsigned int i = 0; i < this->TransformsBuffer.size(); ++ i )
    {
    output << "  <log";
    output << " TimeStampSec=\"" << this->TransformsBuffer[ i ].TimeStampSec << "\"";
    output << " TimeStampNSec=\"" << this->TransformsBuffer[ i ].TimeStampNSec << "\"";
    output << " type=\"transform\"";
    output << " DeviceName=\"" << this->TransformsBuffer[ i ].DeviceName << "\"";
    output << " transform=\"" << this->TransformsBuffer[ i ].Transform << "\"";
    output << " />" << std::endl;
    }
  
  
    // Save messages.
  
  for ( unsigned int i = 0; i < this->MessagesBuffer.size(); ++ i )
    {
    output << "  <log";
    output << " TimeStampSec=\"" << this->MessagesBuffer[ i ].TimeStampSec << "\"";
    output << " TimeStampNSec=\"" << this->MessagesBuffer[ i ].TimeStampNSec << "\"";
    output << " type=\"message\"";
    output << " message=\"" << this->MessagesBuffer[ i ].Message << "\"";
    output << " />" << std::endl;
    }
  
  
  output << "</WorkflowSegmentationLog>" << std::endl;
  output.close();
  this->ClearBuffer();
  
}



void
vtkMRMLWorkflowSegmentationNode
::SaveTrainingParameters( std::string fileName )
{
  std::ofstream output( fileName.c_str() );
  
  if ( ! output.is_open() )
    {
    vtkErrorMacro( "Record file could not be opened!" );
    return;
    }
  
  output << "<WorkflowSegmentationParameters>" << std::endl; 

  output << "  <Parameter Type=\"PrinComps\" Value=\"" << this->trainingParam.PrinComps << "\" />";
  output << "  <Parameter Type=\"Mean\" Value=\"" << this->trainingParam.Mean << "\" />";
  output << "  <Parameter Type=\"Centroids\" Value=\"" << this->trainingParam.Centroids << "\" />";
  output << "  <Parameter Type=\"MarkovPi\" Value=\"" << this->trainingParam.MarkovPi << "\" />";
  output << "  <Parameter Type=\"MarkovA\" Value=\"" << this->trainingParam.MarkovA << "\" />";
  output << "  <Parameter Type=\"MarkovB\" Value=\"" << this->trainingParam.MarkovB << "\" />";

  output << "</WorkflowSegmentationParameters>" << std::endl;

  output.close();
  this->ClearBuffer();
  
}





void
vtkMRMLWorkflowSegmentationNode
::ImportTrainingData( std::string dirName )
{
  // Read all of the xml files from the specified directory
}

void
vtkMRMLWorkflowSegmentationNode
::ImportInputParameters( std::string fileName )
{

  // Create a parser to parse the XML data from TransformRecorderLog
  vtkSmartPointer< vtkXMLDataParser > parser = vtkSmartPointer< vtkXMLDataParser >::New();
  parser->SetFileName( fileName.c_str() );
  parser->Parse();
  
  // Get the root element (and check it exists)
  vtkXMLDataElement* rootElement = parser->GetRootElement();
  if ( ! rootElement )
  {
    return;
  }
  
  int num = rootElement->GetNumberOfNestedElements();  // Number of saved records (including transforms and messages).
  
  for ( int i = 0; i < num; ++ i )
  {

    vtkXMLDataElement* noteElement = rootElement->GetNestedElement( i );
    if ( strcmp( noteElement->GetName(), "Parameter" ) != 0 )
    {
      continue;  // If it's not a "Parameter", jump to the next.
    }

	const char* elementType = noteElement->GetAttribute( "Type" );
	
	std::stringstream ss( std::string( noteElement->GetAttribute( "Value" ) ) );
	double value;
	ss >> value;

    if ( strcmp( elementType, "NumTasks" ) == 0 )
    {
	  this->inputParam.NumTasks = value;
    }
	if ( strcmp( elementType, "Derivative" ) == 0 )
    {
	  this->inputParam.Derivative = value;
    }
	if ( strcmp( elementType, "FilterWidth" ) == 0 )
    {
	  this->inputParam.FilterWidth = value;
    }
	if ( strcmp( elementType, "OrthogonalOrder" ) == 0 )
    {
	  this->inputParam.OrthogonalOrder = value;
    }
	if ( strcmp( elementType, "OrthogonalWindow" ) == 0 )
    {
	  this->inputParam.OrthogonalWindow = value;
    }
	if ( strcmp( elementType, "NumPrinComps" ) == 0 )
    {
	  this->inputParam.NumPrinComps = value;
    }
    if ( strcmp( elementType, "NumCentroids" ) == 0 )
    {
	  this->inputParam.NumCentroids = value;
    }
	if ( strcmp( elementType, "MarkovPseudoScalePi" ) == 0 )
    {
	  this->inputParam.MarkovPseudoScalePi = value;
    }
	if ( strcmp( elementType, "MarkovPseudoScaleA" ) == 0 )
    {
	  this->inputParam.MarkovPseudoScaleA = value;
    }
	if ( strcmp( elementType, "MarkovPseudoScaleB" ) == 0 )
    {
	  this->inputParam.MarkovPseudoScaleB = value;
    }

  }

}


void
vtkMRMLWorkflowSegmentationNode
::ImportTrainingParameters( std::string fileName )
{

  // Create a parser to parse the XML data from TransformRecorderLog
  vtkSmartPointer< vtkXMLDataParser > parser = vtkSmartPointer< vtkXMLDataParser >::New();
  parser->SetFileName( fileName.c_str() );
  parser->Parse();
  
  // Get the root element (and check it exists)
  vtkXMLDataElement* rootElement = parser->GetRootElement();
  if ( ! rootElement )
  {
    return;
  }
  
  int num = rootElement->GetNumberOfNestedElements();  // Number of saved records (including transforms and messages).
  
  for ( int i = 0; i < num; ++ i )
  {

    vtkXMLDataElement* noteElement = rootElement->GetNestedElement( i );
    if ( strcmp( noteElement->GetName(), "Parameter" ) != 0 )
    {
      continue;  // If it's not a "Parameter", jump to the next.
    }

	const char* elementType = noteElement->GetAttribute( "Type" );
	
	std::string value = std::string( noteElement->GetAttribute( "Value" ) );

    if ( strcmp( elementType, "PrinComps" ) == 0 )
    {
	  this->trainingParam.PrinComps = value;
    }
	if ( strcmp( elementType, "Mean" ) == 0 )
    {
	  this->trainingParam.Mean = value;
    }
	if ( strcmp( elementType, "Centroids" ) == 0 )
    {
	  this->trainingParam.Centroids = value;
    }
	if ( strcmp( elementType, "MarkovPi" ) == 0 )
    {
	  this->trainingParam.MarkovPi = value;
    }
	if ( strcmp( elementType, "MarkovA" ) == 0 )
    {
	  this->trainingParam.MarkovA = value;
    }
	if ( strcmp( elementType, "MarkovB" ) == 0 )
    {
	  this->trainingParam.MarkovB = value;
    }

  }


}



void
vtkMRMLWorkflowSegmentationNode
::ClearBuffer()
{
  this->TransformsBuffer.clear();
  this->MessagesBuffer.clear();
  
  if ( this->LastNeedleTransform != NULL )
  {
    this->LastNeedleTransform->Delete();
    this->LastNeedleTransform = NULL;
  }
}

TransformRecord vtkMRMLWorkflowSegmentationNode
::GetTransformAt( int index )
{
  return TransformsBuffer.at(index);
}



void vtkMRMLWorkflowSegmentationNode
::GetTimestamp( int &sec, int &nsec )
{
  clock_t clock1 = clock();
  double seconds = double( clock1 - this->Clock0 ) / CLOCKS_PER_SEC;
  sec = floor( seconds );
  nsec = ( seconds - sec ) * 1e9;    
}



vtkMRMLWorkflowSegmentationNode
::vtkMRMLWorkflowSegmentationNode()
{
  this->HideFromEditorsOff();
  this->SetSaveWithScene( true );
  // this->SetModifiedSinceRead( true );
  
  this->Recording = false;
  
  this->LogFileName = "";
  
  this->Active = true;
  
  // Initialize zero time point.
  this->Clock0 = clock();
  this->IGTLTimeOffsetSeconds = 0.0;
  this->IGTLTimeSynchronized = false;
  
  this->LastNeedleTransform = NULL;
  this->LastNeedleTime = -1.0;
}




vtkMRMLWorkflowSegmentationNode
::~vtkMRMLWorkflowSegmentationNode()
{
  this->ClearObservedTranformNodes();
  
  if ( this->LastNeedleTransform != NULL )
  {
    this->LastNeedleTransform->Delete();
    this->LastNeedleTransform = NULL;
  }
}



void vtkMRMLWorkflowSegmentationNode::WriteXML( ostream& of, int nIndent )
{
  Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);
  
  of << indent << " Recording=\"" << this->Recording << "\"";

  /*
  of << indent << " InputParameterFileName=\"" << this->InputParameterFileName << "\"";
  of << indent << " TrainingParameterFileName=\"" << this->TrainingParameterFileName << "\"";

  of << indent << " NumTasks=\"" << this->inputParam.NumTasks << "\"";
  of << indent << " FilterWidth=\"" << this->inputParam.FilterWidth << "\"";
  of << indent << " OrthogonalOrder=\"" << this->inputParam.OrthogonalOrder << "\"";
  of << indent << " OrthogonalWindow=\"" << this->inputParam.OrthogonalWindow << "\"";
  of << indent << " Derivative=\"" << this->inputParam.Derivative << "\"";
  of << indent << " NumCentroids=\"" << this->inputParam.NumCentroids << "\"";
  of << indent << " NumPrinComps=\"" << this->inputParam.NumPrinComps << "\"";
  of << indent << " MarkovPseudoScalePi=\"" << this->inputParam.MarkovPseudoScalePi << "\"";
  of << indent << " MarkovPseudoScaleA=\"" << this->inputParam.MarkovPseudoScaleA << "\"";
  of << indent << " MarkovPseudoScaleB=\"" << this->inputParam.MarkovPseudoScaleB << "\"";

  of << indent << " PrinComps=\"" << this->trainingParam.PrinComps << "\"";
  of << indent << " Centroids=\"" << this->trainingParam.Centroids << "\"";
  of << indent << " MarkovPi=\"" << this->trainingParam.MarkovPi << "\"";
  of << indent << " MarkovA=\"" << this->trainingParam.MarkovA << "\"";
  of << indent << " MarkovB=\"" << this->trainingParam.MarkovB << "\"";
  */

}



void vtkMRMLWorkflowSegmentationNode::ReadXMLAttributes( const char** atts )
{
  Superclass::ReadXMLAttributes(atts);

  // Read all MRML node attributes from two arrays of names and values
  const char* attName;
  const char* attValue;

  while (*atts != NULL)
  {
    attName  = *(atts++);
    attValue = *(atts++);
    
    // todo: Handle observed transform nodes and connector node.
    
    if ( ! strcmp( attName, "Recording" ) )
      {
      this->SetRecording( StringToBool( std::string( attValue ) ) );
      }
  }

}


//----------------------------------------------------------------------------
void vtkMRMLWorkflowSegmentationNode::Copy( vtkMRMLNode *anode )
{  
  Superclass::Copy( anode );
  vtkMRMLWorkflowSegmentationNode *node = ( vtkMRMLWorkflowSegmentationNode* ) anode;

    // Observers must be removed here, otherwise MRML updates would activate nodes on the undo stack
  
  for ( unsigned int i = 0; i < this->ObservedTransformNodes.size(); ++ i )
    {
    this->ObservedTransformNodes[ i ]->RemoveObservers( vtkMRMLLinearTransformNode::TransformModifiedEvent );
    }
  
  this->SetRecording( node->GetRecording() );
}


//----------------------------------------------------------------------------
void vtkMRMLWorkflowSegmentationNode::UpdateReferences()
{
  Superclass::UpdateReferences();
      // MRML node ID's should be checked. If Scene->GetNodeByID( id ) returns NULL,
    // the reference should be deleted (set to NULL).
  
}



void vtkMRMLWorkflowSegmentationNode::UpdateReferenceID( const char *oldID, const char *newID )
{
  Superclass::UpdateReferenceID( oldID, newID );
  
  /*
  if ( this->ConnectorNodeID && !strcmp( oldID, this->ConnectorNodeID ) )
    {
    this->SetAndObserveConnectorNodeID( newID );
    }
  */
  
  // TODO: This needs to be written for observed transforms.
  
}



void vtkMRMLWorkflowSegmentationNode::UpdateScene( vtkMRMLScene *scene )
{
  Superclass::UpdateScene( scene );
  // this->SetAndObserveConnectorNodeID( this->ConnectorNodeID );
  // TODO: Deal with observed transforms.
}



void vtkMRMLWorkflowSegmentationNode::PrintSelf( ostream& os, vtkIndent indent )
{
  vtkMRMLNode::PrintSelf(os,indent);
  os << indent << "LogFileName: " << this->LogFileName << "\n";
}



void vtkMRMLWorkflowSegmentationNode
::AddObservedTransformNode( const char* TransformNodeID )
{
  if ( TransformNodeID == NULL )
  {
    return;
  }
  
  
    // Check if this ID is already observed.
  
  bool alreadyObserved = false;  
  
  for ( unsigned int i = 0; i < this->ObservedTransformNodeIDs.size(); ++ i )
  {
    if ( strcmp( TransformNodeID, this->ObservedTransformNodeIDs[ i ] ) == 0 )
    {
    alreadyObserved = true;
    }
  }
  
  if ( alreadyObserved == true )
  {
    return;  // No need to add this node again.
  }
  
  
  if ( this->GetScene() )
  {
    this->GetScene()->AddReferencedNodeID( TransformNodeID, this );
  }
  
  
  vtkMRMLLinearTransformNode* transformNode = NULL;
  this->ObservedTransformNodeIDs.push_back( (char*)TransformNodeID );
  vtkMRMLLinearTransformNode* tnode = this->GetObservedTransformNode( TransformNodeID );
  vtkSetAndObserveMRMLObjectMacro( transformNode, tnode );
  this->ObservedTransformNodes.push_back( transformNode );
  if ( tnode )
  {
    tnode->AddObserver( vtkMRMLLinearTransformNode::TransformModifiedEvent, (vtkCommand*)this->MRMLCallbackCommand );
  }
}



void vtkMRMLWorkflowSegmentationNode
::RemoveObservedTransformNode( const char* TransformNodeID )
{
  if ( TransformNodeID == NULL )
  {
    return;
  }
  
  
    // Remove observer, and erase node from the observed node vector.
  
  std::vector< vtkMRMLLinearTransformNode* >::iterator nodeIt;
  for ( nodeIt = this->ObservedTransformNodes.begin();
        nodeIt != this->ObservedTransformNodes.end();
        nodeIt ++ )
  {
    if ( strcmp( TransformNodeID, (*nodeIt)->GetID() ) == 0 )
    {
      (*nodeIt)->RemoveObserver( (vtkCommand*)this->MRMLCallbackCommand );
      vtkSetAndObserveMRMLObjectMacro( *nodeIt, NULL );
      this->ObservedTransformNodes.erase( nodeIt );
    }
  }
  
  
    // Remove node ID and reference.
  
  std::vector< char* >::iterator transformIt;
  for ( transformIt = this->ObservedTransformNodeIDs.begin();
        transformIt != this->ObservedTransformNodeIDs.end();
        transformIt ++ )
  {
    if ( strcmp( TransformNodeID, *transformIt ) == 0 )
    {
      if ( this->GetScene() )
      {
        this->GetScene()->RemoveReferencedNodeID( TransformNodeID, this );
      }
      
      this->ObservedTransformNodeIDs.erase( transformIt );
      break;
    }
  }
}



void vtkMRMLWorkflowSegmentationNode
::ClearObservedTranformNodes()
{
  std::vector< char* >::iterator transformIDIt;
  for ( transformIDIt = this->ObservedTransformNodeIDs.begin();
        transformIDIt != this->ObservedTransformNodeIDs.end();
        transformIDIt ++ )
  {
    this->RemoveObservedTransformNode( *transformIDIt );
  }
}



vtkMRMLLinearTransformNode* vtkMRMLWorkflowSegmentationNode
::GetObservedTransformNode( const char* TransformNodeID )
{
  vtkMRMLLinearTransformNode* node = NULL;
  if ( this->GetScene()
       && TransformNodeID != NULL )
  {
    vtkMRMLNode* snode = this->GetScene()->GetNodeByID( TransformNodeID );
    node = vtkMRMLLinearTransformNode::SafeDownCast( snode );
  }
  return node;
}



void vtkMRMLWorkflowSegmentationNode::ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData )
{
  if ( this->Recording == false )
  {
    return;
  }
  
  
    // Handle modified event of any observed transform node.
  
  std::vector< vtkMRMLLinearTransformNode* >::iterator tIt;
  for ( tIt = this->ObservedTransformNodes.begin();
        tIt != this->ObservedTransformNodes.end();
        tIt ++ )
  {
    if ( *tIt == vtkMRMLLinearTransformNode::SafeDownCast( caller )
         && event == vtkMRMLLinearTransformNode::TransformModifiedEvent )
    {
      vtkMRMLLinearTransformNode* transformNode = vtkMRMLLinearTransformNode::SafeDownCast( caller );
      if ( transformNode != NULL )
      {
        this->AddNewTransform( transformNode->GetID() );
      }
    }
  }
}



void vtkMRMLWorkflowSegmentationNode::RemoveMRMLObservers()
{
  for ( unsigned int i = 0; i < this->ObservedTransformNodes.size(); ++ i )
  {
    this->ObservedTransformNodes[ i ]->RemoveObservers( vtkMRMLLinearTransformNode::TransformModifiedEvent );
  }
}



/**
 * @param selections Should contain as many elements as the number of incoming
 *        transforms throught the active connector. Order follows the order in
 *        the connector node. 0 means transform is not tracked, 1 means it's tracked.
 */
void vtkMRMLWorkflowSegmentationNode::SetTransformSelections( std::vector< int > selections )
{
  this->TransformSelections.clear();
  
  for ( unsigned int i = 0; i < selections.size(); ++ i )
  {
    this->TransformSelections.push_back( selections[ i ] );
  }


}




void vtkMRMLWorkflowSegmentationNode::CustomMessage( std::string message, int sec, int nsec )
{
  if ( sec == -1  &&  nsec == -1 )
  {
    this->GetTimestamp( sec, nsec );
  }
  
  MessageRecord rec;
    rec.Message = message;
    rec.TimeStampSec = sec;
    rec.TimeStampNSec = nsec;
  
  this->MessagesBuffer.push_back( rec );

}



unsigned int vtkMRMLWorkflowSegmentationNode::GetTransformsBufferSize()
{
  return this->TransformsBuffer.size();

}



unsigned int vtkMRMLWorkflowSegmentationNode::GetMessagesBufferSize()
{
  return this->MessagesBuffer.size();
}



double vtkMRMLWorkflowSegmentationNode::GetTotalTime()
{
  double totalTime = 0.0;
  unsigned int n = this->TransformsBuffer.size();
  
  if ( n < 2 )
  {
    return totalTime;
  }
  
  double diffSec = double( this->TransformsBuffer[ n - 1 ].TimeStampSec - this->TransformsBuffer[ 0 ].TimeStampSec );
  double diffNSec = double( this->TransformsBuffer[ n - 1 ].TimeStampNSec - this->TransformsBuffer[ 0 ].TimeStampNSec );
  
  totalTime = diffSec + 1.0e-9 * diffNSec;
  
  return totalTime;
}



// TODO: Move these to logic, and implement properly
std::string vtkMRMLWorkflowSegmentationNode::GetCurrentTask()
{
  unsigned int n = this->TransformsBuffer.size();
  if ( n < 1 )
  {
    return "";
  }
  //return TransformsBuffer[ n - 1 ].Task;
  std::string currTask = "Task 1 (Translation)";
  if ( this->GetTotalTime() > 10.6 )
	  currTask = "Task 2 (Rotation)";
  if ( this->GetTotalTime() > 14.0 )
	  currTask = "Task 3 (Insertion)";
  if ( this->GetTotalTime() > 18.4 )
	  currTask = "Task 5 (Retraction)";
  if ( this->GetTotalTime() > 19.2 )
	  currTask = "Task 3 (Insertion)";
  if ( this->GetTotalTime() > 21.0 )
	  currTask = "Task 4 (Verification)";
  if ( this->GetTotalTime() > 21.4 )
	  currTask = "Task 5 (Retraction)";
  if ( this->GetTotalTime() > 22.0 )
	  currTask = "Task 1 (Translation)";

  return currTask;
}


std::string vtkMRMLWorkflowSegmentationNode::GetCurrentInstruction()
{
  unsigned int n = this->TransformsBuffer.size();
  if ( n < 1 )
  {
    return "";
  }
  //return TransformsBuffer[ n - 1 ].Task;
  std::string currTask = "Move needle-tip between L3-L4 vertebrae.";
  if ( this->GetTotalTime() > 10.6 )
	  currTask = "Angle needle 15 degrees medially.";
  if ( this->GetTotalTime() > 14.0 )
	  currTask = "Insert needle into subarachnoid space.";
  if ( this->GetTotalTime() > 18.4 )
	  currTask = "Remove needle from tissue.";
  if ( this->GetTotalTime() > 19.2 )
	  currTask = "Insert needle into subarachnoid space.";
  if ( this->GetTotalTime() > 21.0 )
	  currTask = "Remove needle stylet; verify CSF.";
  if ( this->GetTotalTime() > 21.4 )
	  currTask = "Remove needle from tissue.";
  if ( this->GetTotalTime() > 22.0 )
	  currTask = "Move needle-tip between L3-L4 vertebrae.";

  return currTask;
}


std::string vtkMRMLWorkflowSegmentationNode::GetNextTask()
{
  unsigned int n = this->TransformsBuffer.size();
  if ( n < 1 )
  {
    return "";
  }
  //return TransformsBuffer[ n - 1 ].Task;
  std::string currTask = "Task 2 (Rotation)";
  if ( this->GetTotalTime() > 10.6 )
	  currTask = "Task 3 (Insertion)";
  if ( this->GetTotalTime() > 14.0 )
	  currTask = "Task 4 (Verification)";
  if ( this->GetTotalTime() > 18.4 )
	  currTask = "Task 1 (Translation)";
  if ( this->GetTotalTime() > 19.2 )
	  currTask = "Task 4 (Verification)";
  if ( this->GetTotalTime() > 21.0 )
	  currTask = "Task 5 (Retraction)";
  if ( this->GetTotalTime() > 21.4 )
	  currTask = "Task 1 (Translation)";
  if ( this->GetTotalTime() > 22.0 )
	  currTask = "Task 2 (Rotation)";

  return currTask;
}


std::string vtkMRMLWorkflowSegmentationNode::GetNextInstruction()
{
  unsigned int n = this->TransformsBuffer.size();
  if ( n < 1 )
  {
    return "";
  }
  //return TransformsBuffer[ n - 1 ].Task;
  std::string currTask = "Angle needle 15 degrees medially.";
  if ( this->GetTotalTime() > 10.6 )
	  currTask = "Insert needle into subarachnoid space.";
  if ( this->GetTotalTime() > 14.0 )
	  currTask = "Remove needle stylet; verify CSF.";
  if ( this->GetTotalTime() > 18.4 )
	  currTask = "Move needle-tip between L3-L4 vertebrae.";
  if ( this->GetTotalTime() > 19.2 )
	  currTask = "Remove needle stylet; verify CSF.";
  if ( this->GetTotalTime() > 21.0 )
	  currTask = "Remove needle from tissue.";
  if ( this->GetTotalTime() > 21.4 )
	  currTask = "Move needle-tip between L3-L4 vertebrae.";
  if ( this->GetTotalTime() > 22.0 )
	  currTask = "Angle needle 15 degrees medially.";

  return currTask;
}




void vtkMRMLWorkflowSegmentationNode::SetRecording( bool newState )
{
  this->Recording = newState;
  if ( this->Recording )
  {
    this->InvokeEvent( this->RecordingStartEvent, NULL );
  }
  else
  {
    this->InvokeEvent( this->RecordingStopEvent, NULL );
  }
}



void vtkMRMLWorkflowSegmentationNode::AddNewTransform( const char* TransformNodeID )
{
  int sec = 0;
  int nsec = 0;
  vtkSmartPointer< vtkMatrix4x4 > m = vtkSmartPointer< vtkMatrix4x4 >::New();
  std::string deviceName;
  
  this->GetTimestamp( sec, nsec );
  
  
    // Get the new transform matrix.
  
  vtkMRMLLinearTransformNode* ltn = this->GetObservedTransformNode( TransformNodeID );
  
  if ( ltn != NULL )
  {
    m->DeepCopy( ltn->GetMatrixTransformToParent() );
  }
  else
  {
    vtkErrorMacro( "Transform node not found." );
  }
  
    
    // Get the device name for the new transform.
  
  deviceName = std::string( ltn->GetName() );
  

    // Record the transform.
  
  std::stringstream mss;
  for ( int row = 0; row < 4; ++ row )
  {
    for ( int col = 0; col < 4; ++ col )
    {
      mss << m->GetElement( row, col ) << " ";
    }
  }
  
  TransformRecord rec;
    rec.DeviceName = deviceName;
    rec.TimeStampSec = sec;
    rec.TimeStampNSec = nsec;
    rec.Transform = mss.str();
	rec.Task = "Task 1";
  
  if ( this->Recording == false )
  {
    return;
  }
  
  
    // Look for the most recent value of this transform.
    // If the value hasn't changed, we don't record.
  
  std::vector< TransformRecord >::iterator trIt;
  trIt = this->TransformsBuffer.end();
  bool duplicate = false;
  while ( trIt != this->TransformsBuffer.begin() )
  {
    trIt --;
    if ( rec.DeviceName.compare( (*trIt).DeviceName ) == 0 )
    {
      if ( rec.Transform.compare( (*trIt).Transform ) == 0 )
      {
        duplicate = true;
      }
      break;
    }
  }
  
  if ( duplicate == false )
  {
    this->TransformsBuffer.push_back( rec );
  }
}
