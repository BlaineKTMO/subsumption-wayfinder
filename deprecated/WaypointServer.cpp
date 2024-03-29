#include <WaypointServer.h>

/**
* Gets gps coordinates from text file.
* 
* @param file_name name of file to read from.
*/
void WaypointServer::readGoals(std::string file_name) {
    std::string path = ros::package::getPath("subsumption_model").append("/src/");
    std::ifstream file;
    std::string line;

    // Guard
    if(file_name.length() < 0)
        throw std::invalid_argument("Goals text file name invalid.");

    path.append(file_name);

    // Pass empty vector
    file.open(path, std::ios::in);
    if(!file.is_open())
        return;

    ROS_INFO("Reading goals . . .");

    // Fill vector
    while(getline(file, line))
    {
        geometry_msgs::Point waypoint;

        std::istringstream iss(line);
        std::string token;

        std::getline(iss, token, ';');
        waypoint.x = std::stod(token);

        std::getline(iss, token, ';');
        waypoint.y = std::stod(token);

        std::getline(iss, token, ';');
        waypoint.z = std::stod(token);
        
        wpt_vec.push_back(waypoint);
    }

    goal_count = wpt_vec.size();

    file.close();

    ROS_INFO("Goals read successfully!");
}

void WaypointServer::wayfind_callback(const std_msgs::BoolConstPtr& status) {
    wayfind_complete = true;
    wayfind_status = true;
    ROS_INFO("Wayfind status callback triggered");
}
void WaypointServer::advance() {
    if ( counter >= goal_count )
        return;

    counter = counter + 1;
}

void WaypointServer::recede() {
    if ( counter <= 0 )
        return;

    counter = counter - 1;
}

void WaypointServer::start() {
    if ( wpt_vec.empty() ) {
        ROS_INFO("No goals . . .");
        return;
    }

    if ( counter >= goal_count ) {
        ROS_INFO("All goals reached!");
        return;
    }

    while(counter <= goal_count) {
        geometry_msgs::PoseStamped goal;

        // Initialize global waypoint.
        goal.header.frame_id = "map";
        goal.header.stamp = ros::Time(0);
        goal.pose.position = wpt_vec.at(counter);
        goal.pose.position.z = 0;

        goal.pose.orientation.x = 0;
        goal.pose.orientation.y = 0;
        goal.pose.orientation.z = 0;
        goal.pose.orientation.w = 1;

        ROS_INFO("Sending goal %d: %f, %f",
                 counter,
                 goal.pose.position.x,
                 goal.pose.position.y);

        // goal_pub.publish(goal);
        // 
        // std_msgs::BoolConstPtr boolPtr = ros::topic::waitForMessage<std_msgs::Bool>("/wayfindStatus", n);
        // if (boolPtr->data)
        // {
        //     ROS_INFO("Reached global goal!");
        //     advance();
        // }
        // else
        // {
        //     ROS_INFO("Did not reach goal . . .");
        //     recede();
        // }

        while(!wayfind_complete) {
            goal_pub.publish(goal);
            
            rate.sleep();
        }
        ROS_INFO("Reached Goal");

        // if (wayfind_status)

        advance();

        rate.sleep();
    }
}

int main(int argc, char * argv[]) {

    ros::init(argc, argv, "WaypointServer");
    WaypointServer waypointServer;

    waypointServer.readGoals("goals_karina.txt");
    waypointServer.start();
    
    ros::spin();
    
    return 0;
}
