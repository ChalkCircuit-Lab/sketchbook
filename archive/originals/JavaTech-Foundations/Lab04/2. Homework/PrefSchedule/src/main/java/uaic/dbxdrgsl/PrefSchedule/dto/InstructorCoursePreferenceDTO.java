package uaic.dbxdrgsl.PrefSchedule.dto;

import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.Data;
import lombok.NoArgsConstructor;
import com.fasterxml.jackson.annotation.JsonProperty;

/**
 * DTO for creating/updating instructor course preferences
 */
@Data
@NoArgsConstructor
@AllArgsConstructor
@Builder
public class InstructorCoursePreferenceDTO {

    @JsonProperty("optional_course_id")
    private Long optionalCourseId;

    @JsonProperty("optional_course_code")
    private String optionalCourseCode;

    @JsonProperty("compulsory_course_code")
    private String compulsoryCourseCode;

    @JsonProperty("percentage")
    private double percentage;
}
