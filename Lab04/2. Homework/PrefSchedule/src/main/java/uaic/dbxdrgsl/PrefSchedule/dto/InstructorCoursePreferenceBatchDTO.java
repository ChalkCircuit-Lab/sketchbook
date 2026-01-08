package uaic.dbxdrgsl.PrefSchedule.dto;

import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.Data;
import lombok.NoArgsConstructor;
import com.fasterxml.jackson.annotation.JsonProperty;
import java.util.List;

/**
 * DTO for batch creating instructor course preferences
 */
@Data
@NoArgsConstructor
@AllArgsConstructor
@Builder
public class InstructorCoursePreferenceBatchDTO {

    @JsonProperty("optional_course_id")
    private Long optionalCourseId;

    @JsonProperty("preferences")
    private List<PreferenceItem> preferences;

    @Data
    @NoArgsConstructor
    @AllArgsConstructor
    @Builder
    public static class PreferenceItem {
        @JsonProperty("compulsory_course_code")
        private String compulsoryCourseCode;

        @JsonProperty("percentage")
        private double percentage;
    }
}
