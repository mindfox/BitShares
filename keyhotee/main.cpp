#include <bts/application.hpp>
#include <fc/exception/exception.hpp>
#include <fc/log/logger.hpp>
#include <fc/thread/thread.hpp>
#include <fc/io/fstream.hpp>
#include <fc/io/json.hpp>
#include <fc/log/logger.hpp>
#include <fc/reflect/variant.hpp>

#include "profile_wizard/ProfileWizard.hpp"

#include <QApplication>
#include <QStandardPaths>

fc::future<void> app_loop;

void process_qt_events()
{
   while( !app_loop.canceled() )
   {
      QCoreApplication::instance()->sendPostedEvents();
      QCoreApplication::instance()->processEvents();
      fc::usleep( fc::microseconds( 1000*20 ) );
   }
}

bts::application_config load_config()
{ try {
     auto qdatadir     = QStandardPaths::writableLocation( QStandardPaths::DataLocation );
     auto data_dir     = fc::path( qdatadir.toStdString() );
     fc::create_directories(data_dir);
     auto config_file  = data_dir / "config.json";
     ilog( "config_file: ${file}", ("file",config_file) );
     if( !fc::exists( config_file ) )
     {
        fc::ofstream out( config_file );
        out << fc::json::to_pretty_string( bts::application_config() );
     }
     return fc::json::from_file( config_file ).as<bts::application_config>();
} FC_RETHROW_EXCEPTIONS( warn, "") }


void start_profile_creation_wizard(const bts::application_ptr& btsapp);
void display_login(const bts::application_ptr& btsapp);

void fc_main()
{
   auto btsapp     = std::make_shared<bts::application>();
   auto app_config = load_config();
   btsapp->configure( app_config );

   if( btsapp->has_profile() )
   {
      display_login(btsapp);
   }
   else
   {
      start_profile_creation_wizard(btsapp);
   }
   while( !app_loop.canceled() )
   {
     fc::usleep( fc::microseconds(1000*100) );
   }
}

int main( int argc, char** argv )
{
  try {
     QApplication app(argc,argv); 

     app.setOrganizationDomain( "invictus-innovations.com" );
     app.setOrganizationName( "Invictus Innovations, Inc" );
     app.setApplicationName( "Keyhotee" );

     app_loop = fc::async( [=]{ fc_main(); } );

     app.connect( &app, &QCoreApplication::aboutToQuit, [](){ app_loop.cancel(); } );

     while( !app_loop.ready() )
     {
        QCoreApplication::instance()->sendPostedEvents();
        QCoreApplication::instance()->processEvents();
        fc::usleep( fc::microseconds( 1000*20 ) );
     }
     return 0;
  } 
  catch ( const fc::exception& e )
  {
     elog( "${e}", ("e", e.to_detail_string() ) );
  }
  return -1;
}

void start_profile_creation_wizard( const bts::application_ptr& btsapp )
{
   // TODO: figure out memory management here..
   auto pro_wiz = new ProfileWizard(nullptr, btsapp);  
   pro_wiz->show();
}

void display_login(const bts::application_ptr& btsapp)
{
}