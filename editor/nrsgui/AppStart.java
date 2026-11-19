package nrsgui;

import java.util.logging.*;


public class AppStart
{
    //------------------------------------------------------------------------
    // M A I N
    //------------------------------------------------------------------------



    /**
     * The main method for invoking the GUI. This class creates an
     * instance of <code>AppManager</code> which is responsible for
     * managing the application.
     *
     * @param args a <code>String[]</code> value holding the commandline
     * arguments the user provided when invoking this application.
     */
    public static void main(String[] args)
    {
        // The main application manager
        AppManager appManager;

        Logger logger = Logger.getLogger("nrsgui");
        
        logger.info(Constants.APPLICATION_NAME
                    + " started. Version "
                    + Constants.APPLICATION_VERSION);

        
        appManager = new AppManager();

        appManager.startup(args);
    }

}



